#include "gui/mainwindow.h"
#include "can/parser/canparserworker.h"
#include "can/transport/CannelloniFrame.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <QMutexLocker>
#include <algorithm> // Для std::clamp
#include <can/canbus.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    label(new ClickableLabel(this)),
    localMessageQueue{1000}
{
    initUI();
    initVideoThread();
    initCAN();

    //timer->start(30); // 30 мс інтервал оновлення (приблизно 33.3 кадри/сек)
}

MainWindow::~MainWindow() {
    //cap.release();
    if (videoWorker) videoWorker->stop();
    if (videoThread) {
        videoThread->quit();
        videoThread->wait();
    }

    // Відправити нульові швидкості, щоб гімбал зупинився при закритті програми
}

void MainWindow::initUI()
{
    // Display widget
    label->setFixedSize(1920, 1080);
    label->setAlignment(Qt::AlignCenter);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(label);
    setCentralWidget(central);

    // User click → tracker init
    connect(label, &ClickableLabel::clicked, this, &MainWindow::onLabelClicked);

    // Frame update timer
    //connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
}


void MainWindow::onLabelClicked(QPoint pos) {
    qDebug() << "Clicked on screen:" << pos;

    cv::Mat initFrame;
    {
        QMutexLocker locker(&frameMutex);
        if (lastFrame.empty()) {
            qDebug() << "No frame available for tracker init";
            return;
        }
        initFrame = lastFrame.clone();
    }

    int frameW = initFrame.cols;
    int frameH = initFrame.rows;

    // QLabel size
    int labelW = label->width();
    int labelH = label->height();

    // Compute scale while preserving aspect ratio (same as in updateFrame)
    double scale = std::min((double)labelW / frameW, (double)labelH / frameH);
    int displayedW = frameW * scale;
    int displayedH = frameH * scale;

    // Compute offsets (if video is centered with borders)
    int offsetX = (labelW - displayedW) / 2;
    int offsetY = (labelH - displayedH) / 2;

    // Check if click is inside the displayed video area
    if (pos.x() < offsetX || pos.x() > offsetX + displayedW ||
        pos.y() < offsetY || pos.y() > offsetY + displayedH) {
        qDebug() << "Click outside of video area.";
        return;
    }

    // Map click to video coordinates
    double videoX = (pos.x() - offsetX) / scale;
    double videoY = (pos.y() - offsetY) / scale;

    // Safe ROI (100x100 px)
    double roiSize = 50;
    double x = std::clamp(videoX - roiSize / 2, 0.0, (double)frameW - roiSize);
    double y = std::clamp(videoY - roiSize / 2, 0.0, (double)frameH - roiSize);
    trackingROI = cv::Rect2d(x, y, roiSize, roiSize);

    tracker = cv::TrackerCSRT::create();
    tracker->init(initFrame, trackingROI);
    trackingActive = true;

    qDebug() << "[TRACKING] Initialized at (" << videoX << "," << videoY
             << ") ROI:" << trackingROI.x << "," << trackingROI.y;

    // Reset PID terms
    integralYaw = integralPitch = previousErrorYaw = previousErrorPitch = 0.0f;
    //siyi.sendSpeeds(0.0f, 0.0f, 0.0f);
}




void MainWindow::initVideoThread()
{
    videoThread = new QThread(this);
    videoWorker = new VideoWorker();

    videoWorker->moveToThread(videoThread);


    // open video source
    //cap.open("rtsp://192.168.144.25:8554/main.264", cv::CAP_FFMPEG);
    //cap.open("/dev/video7");
    videoWorker->setRtspUrl("/dev/video7");
    //videoWorker->setRtspUrl("/home/lps/2025-10-14 14-52-14.mp4");

    connect(videoThread, &QThread::started, videoWorker, &VideoWorker::start);
    connect(this, &MainWindow::destroyed, videoWorker, &VideoWorker::stop);

    //connect(videoWorker, &VideoWorker::frameReady, this, &MainWindow::onFrameReady, Qt::QueuedConnection);
    //Таймер відображення
    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, [this]() {
        if (!videoWorker) return;

        cv::Mat frameBgr;
        quint64 fid = 0; // номер останнього кадру
        qint64 tsMs = 0;

        if (!videoWorker->tryGetLatestFrame(frameBgr, fid, tsMs)) return;

        // save for click/ROI
        {
            QMutexLocker locker(&frameMutex);
            lastFrame = frameBgr; // frameBgr already clone() from worker getter
        }

        // --- CONTROL: draw fps + latency + dropped estimate ---
        if (!uiFpsT.isValid()) uiFpsT.start();
        uiCnt++;

        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const qint64 latencyMs = (tsMs > 0) ? (nowMs - tsMs) : -1; //затримка кадру в мс(вік кадру між моментом “worker записав latest” і моментом “UI його взяв і порахував now”)

        qint64 dropped = 0; //пропуски кадрів
        if (lastDrawId != 0 && fid > lastDrawId) dropped = (qint64)(fid - lastDrawId - 1);
        lastDrawId = fid;

        if (uiFpsT.elapsed() >= 1000) {
            qDebug() << "[UI draw fps]" << uiCnt
                     << "lat(ms)=" << latencyMs
                     << "drop~" << dropped
                     << "fid=" << fid; //номер кадру який відобразився
            uiCnt = 0;
            uiFpsT.restart();
        }
        // --- END CONTROL ---

        // display
        cv::Mat rgb;
        cv::cvtColor(frameBgr, rgb, cv::COLOR_BGR2RGB);
        QImage img(rgb.data, rgb.cols, rgb.rows, (int)rgb.step, QImage::Format_RGB888);
        label->setPixmap(QPixmap::fromImage(img.copy()));
    });
    //displayTimer->start(33); // 30 Hz UI
    displayTimer->start(16); // 60 Hz UI

    connect(videoWorker, &VideoWorker::status, this, &MainWindow::onVideoStatus, Qt::QueuedConnection);

    // cleanup
    connect(videoThread, &QThread::finished, videoWorker, &QObject::deleteLater);

    videoThread->start();

}

/*
void MainWindow::initVideo()
{

    // open video source
    //cap.open("rtsp://192.168.144.25:8554/main.264", cv::CAP_FFMPEG);
    //cap.open("/dev/video7");

    cap.open("/home/lps/2025-10-14 14-52-14.mp4");
    if (!cap.isOpened()) {
        qDebug() << "Failed to open RTSP stream. Check camera IP/port or network connection.";
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    videoSize = QSize(1920, 1080);
}
*/

// void MainWindow::onFrameReady(const QImage &img)
// {
//     static QElapsedTimer fpsT;
//     static int fpsCnt = 0;
//     if (!fpsT.isValid()) fpsT.start();

//     fpsCnt++;
//     if (fpsT.elapsed() >= 1000) {
//         qDebug() << "[UI draw fps]" << fpsCnt;
//         fpsCnt = 0;
//         fpsT.restart();
//     }


//     // конвертація назад у cv::Mat
//     cv::Mat frame(img.height(),
//                   img.width(),
//                   CV_8UC3,
//                   const_cast<uchar*>(img.bits()),
//                   img.bytesPerLine());

//     QMutexLocker locker(&frameMutex);
//     lastFrame = frame.clone();   // зберігаємо копію

//     // якщо label — ClickableLabel
//     label->setPixmap(QPixmap::fromImage(img));
// }

void MainWindow::onVideoStatus(const QString &txt)
{
    qDebug() << "[Video]" << txt;
}

void MainWindow::initCAN()
{
    qDebug() << "[MainWindow] thread =" << QThread::currentThread();
    canBus = new CanBus(this);

    // RX callback
    connect(canBus, &CanBus::packetReceived,
            this, &MainWindow::handleCANPacket);

    setupParserThread();
    setupQueueTransfer();

    // Стартуємо прийом пакету
    canBus->startReceiving();

}

void MainWindow::handleCANPacket(const QByteArray &packetData)
{
    try {
        // decode Cannelloni frame
        CannelloniFrame frame(packetData);

        QMutexLocker locker(&queueMutex);// protect RX queue

        activeRX = 50;

        std::queue<std::vector<uint8_t>> frameQueue = frame.GetMessageQueue();

        // move messages to circular buffer
        while (!frameQueue.empty()) {

            localMessageQueue.push(frameQueue.front());

            frameQueue.pop();
        }

    } catch (const std::exception &e) {

        qWarning() << "Error parsing CannelloniFrame:" << e.what();
    }
}

void MainWindow::setupParserThread()
{
    parserWorker = new CANParserWorker();

    parserThread = new QThread();

    parserWorker->moveToThread(parserThread);

    connect(parserThread,
            &QThread::started,
            parserWorker,
            &CANParserWorker::process);

    parserThread->start();
}

void MainWindow::setupQueueTransfer()
{
    QTimer *queueTransferTimer = new QTimer(this);

    connect(queueTransferTimer, &QTimer::timeout,
            this, &MainWindow::transferQueue);

    queueTransferTimer->start(10); // Кожні 10 мс перевіряє чергу
}

void MainWindow::transferQueue()
{
    QMutexLocker locker(&queueMutex);

    std::vector<uint8_t> msg;

    const int maxMsgs = 100; // avoid long blocking

    for (int i = 0; i < maxMsgs && localMessageQueue.pop(msg); ++i)
    {
        parserWorker->enqueueMessage(msg);
    }
}
