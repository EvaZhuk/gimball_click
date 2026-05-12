#include "gui/mainwindow.h"
#include "can/parser/canparserworker.h"
#include "can/transport/CannelloniFrame.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QMutexLocker>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    label(new ClickableLabel(this)),
    localMessageQueue{1000}
{
    initUI();
    initVideoThread();
    initCAN();
}

MainWindow::~MainWindow()
{
    if (displayTimer)
        displayTimer->stop();

    if (udpStreamer)
        udpStreamer->stop();

    if (videoWorker)
        videoWorker->stop();

    if (videoThread) {
        videoThread->quit();
        videoThread->wait();
    }

    if (parserThread) {
        parserThread->quit();
        parserThread->wait();
    }
}

void MainWindow::initUI()
{
    label->setFixedSize(1280, 720);
    label->setAlignment(Qt::AlignCenter);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(label);
    setCentralWidget(central);

    connect(label, &ClickableLabel::clicked,
            this, &MainWindow::onLabelClicked);


}

bool MainWindow::mapLabelPointToFrame(const QPoint &pos, const cv::Mat &frame, cv::Point &framePt)
{
    if (frame.empty())
        return false;

    const int frameW = frame.cols;
    const int frameH = frame.rows;

    const int labelW = label->width();
    const int labelH = label->height();

    const double scale = std::min(double(labelW) / frameW, double(labelH) / frameH);
    const int displayedW = int(frameW * scale);
    const int displayedH = int(frameH * scale);

    const int offsetX = (labelW - displayedW) / 2;
    const int offsetY = (labelH - displayedH) / 2;

    if (pos.x() < offsetX || pos.x() >= offsetX + displayedW ||
        pos.y() < offsetY || pos.y() >= offsetY + displayedH) {
        return false;
    }

    const int x = int((pos.x() - offsetX) / scale);
    const int y = int((pos.y() - offsetY) / scale);

    framePt.x = std::clamp(x, 0, frameW - 1);
    framePt.y = std::clamp(y, 0, frameH - 1);
    return true;
}

// локальний клік мишкою
void MainWindow::onLabelClicked(QPoint pos)
{
    qDebug() << "Clicked on screen:" << pos;

    cv::Mat frame;
    {
        QMutexLocker locker(&frameMutex);
        if (lastFrame.empty()) {
            qDebug() << "No frame available"
                        "";
            return;
        }
        frame = lastFrame.clone();
    }

    cv::Point framePt;
    if (!mapLabelPointToFrame(pos, frame, framePt)) {
        qDebug() << "Click outside displayed video area";
        return;
    }

    qDebug() << "[CLICK] local click frame point =" << framePt.x << framePt.y;

    startTrackingAtPoint(framePt.x, framePt.y);
}


void MainWindow::showFrameOnScreen(const cv::Mat &frameBgr)
{
    if (frameBgr.empty())
        return;

    cv::Mat rgb;
    cv::cvtColor(frameBgr, rgb, cv::COLOR_BGR2RGB);

    QImage img(rgb.data,
               rgb.cols,
               rgb.rows,
               static_cast<int>(rgb.step),
               QImage::Format_RGB888);

    label->setPixmap(QPixmap::fromImage(img.copy()));
}

// Отримання кліку по КАН
void MainWindow::onCapturePointReceived(quint16 x, quint16 y)
{
    qDebug() << "[CAN RX] capture point =" << x << y;
    startTrackingAtPoint(static_cast<int>(x), static_cast<int>(y));
}

// Отримання нормалізованих координат кліку по КАН
void MainWindow::onCapturePointNormalizedReceived(float nx, float ny)
{
    qDebug() << "[CAN RX] normalized capture point =" << nx << ny;
    startTrackingNormalized(nx, ny);
}

// Спільна функція запуску трекінгу
void MainWindow::startTrackingAtPoint(int xCenter, int yCenter)
{
    cv::Mat initFrame;
    {
        QMutexLocker locker(&frameMutex);
        if (lastFrame.empty()) {
            qDebug() << "No frame available for tracker init";
            return;
        }
        initFrame = lastFrame.clone();
    }

    if (xCenter < 0 || yCenter < 0 ||
        xCenter >= initFrame.cols || yCenter >= initFrame.rows) {
        qDebug() << "[TRACKING] point out of frame:"
                 << xCenter << yCenter
                 << "frame =" << initFrame.cols << initFrame.rows;
        return;
    }

    const int roiW = 80;
    const int roiH = 80;

    int x = xCenter - roiW / 2;
    int y = yCenter - roiH / 2;

    x = std::clamp(x, 0, std::max(0, initFrame.cols - roiW));
    y = std::clamp(y, 0, std::max(0, initFrame.rows - roiH));

    trackingROI = cv::Rect(x, y, roiW, roiH);

    tracker.release();
    tracker = cv::TrackerCSRT::create();

    if (!tracker) {
        qDebug() << "[TRACKING] tracker create failed";
        trackingActive = false;
        return;
    }

    try {
        tracker->init(initFrame, trackingROI);
        trackingActive = true;
    } catch (const cv::Exception &e) {
        qDebug() << "[TRACKING] init exception:" << e.what();
        trackingActive = false;
        return;
    }

    integralYaw = 0.0f;
    integralPitch = 0.0f;
    previousErrorYaw = 0.0f;
    previousErrorPitch = 0.0f;

    qDebug() << "[TRACKING] init ok"
             << "center =" << xCenter << yCenter
             << "roi =" << trackingROI.x << trackingROI.y
             << trackingROI.width << trackingROI.height;
}

void MainWindow::drawTrackingOverlay(cv::Mat &frame, bool ok)
{
    if (frame.empty())
        return;

    if (trackingActive && ok) {
        cv::rectangle(frame, trackingROI, cv::Scalar(0, 255, 0), 2);

        cv::Point center(trackingROI.x + trackingROI.width / 2,
                         trackingROI.y + trackingROI.height / 2);

        cv::drawMarker(frame, center, cv::Scalar(0, 255, 0),
                       cv::MARKER_CROSS, 20, 1);

        // cv::putText(frame, "TRACK",
        //             cv::Point(trackingROI.x, std::max(20, trackingROI.y - 8)),
        //             cv::FONT_HERSHEY_SIMPLEX, 0.8,
        //             cv::Scalar(0, 255, 0), 2, cv::LINE_AA);

        QString text = QString("x=%1 y=%2").arg(center.x).arg(center.y);

        cv::putText(frame, text.toStdString(),
                    cv::Point(trackingROI.x, std::max(15, trackingROI.y - 5)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    } else if (trackingActive && !ok) {
        cv::putText(frame, "TRACK LOST",
                    cv::Point(30, 40),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0,
                    cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // screen center marker
    cv::Point screenCenter(frame.cols / 2, frame.rows / 2);
    //cv::drawMarker(frame, screenCenter, cv::Scalar(255, 255, 0),
    //               cv::MARKER_CROSS, 30, 2);
}

void MainWindow::updateTrackerAndOverlay(cv::Mat &frame)
{
    bool ok = false;

    if (trackingActive && tracker) {
        ok = tracker->update(frame, trackingROI);

        if (!ok) {
            // можна або вимкнути trackingActive, або лишити статус "TRACK LOST"
            // тут лишаю активним, щоб було видно статус втрати
        }
    }

    drawTrackingOverlay(frame, ok);
}

void MainWindow::initVideoThread()
{
    videoThread = new QThread(this);
    videoWorker = new VideoWorker();
    udpStreamer = new UdpStreamer(this);

    videoWorker->moveToThread(videoThread);

    // choose source
    videoWorker->setSource("/dev/video0");
    // videoWorker->setSource("/home/lps/2025-10-14 14-52-14.mp4");
    // videoWorker->setSource("rtsp://192.168.144.25:8554/main.264");
    // videoWorker->setSource("v4l2src device=/dev/video0 ! videoconvert ! video/x-raw,format=BGR ! appsink drop=1 sync=false");

    connect(videoThread, &QThread::started,
            videoWorker, &VideoWorker::start);

    connect(this, &MainWindow::destroyed,
            videoWorker, &VideoWorker::stop);

    connect(videoWorker, &VideoWorker::status,
            this, &MainWindow::onVideoStatus, Qt::QueuedConnection);

    connect(videoThread, &QThread::finished,
            videoWorker, &QObject::deleteLater);

    displayTimer = new QTimer(this);

    connect(displayTimer, &QTimer::timeout, this, [this]() {
        if (!videoWorker)
            return;

        cv::Mat frameBgr;
        quint64 fid = 0;
        qint64 tsMs = 0;

        if (!videoWorker->tryGetLatestFrame(frameBgr, fid, tsMs))
            return;

        // tracker update + ROI draw on FULL frame
        updateTrackerAndOverlay(frameBgr);

        // store annotated frame for possible next click
        {
            QMutexLocker locker(&frameMutex);
            lastFrame = frameBgr.clone();
        }

        if (!uiFpsT.isValid())
            uiFpsT.start();

        uiCnt++;

        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const qint64 latencyMs = (tsMs > 0) ? (nowMs - tsMs) : -1;

        qint64 dropped = 0;
        if (lastDrawId != 0 && fid > lastDrawId)
            dropped = static_cast<qint64>(fid - lastDrawId - 1);
        lastDrawId = fid;

        if (uiFpsT.elapsed() >= 1000) {
            qDebug() << "[UI draw fps]" << uiCnt
                     << "lat(ms)=" << latencyMs
                     << "drop~" << dropped
                     << "fid=" << fid;
            uiCnt = 0;
            uiFpsT.restart();
        }

        // init streamer lazily from actual frame size
        if (udpStreamer && !udpStreamer->isReady()) {
            udpStreamer->init("192.168.144.15", 5601, frameBgr.cols, frameBgr.rows, 30);
        }

        // send FULL annotated frame to UDP
        if (udpStreamer && udpStreamer->isReady()) {
            udpStreamer->sendFrame(frameBgr);
        }

        // display same annotated frame in UI
        // cv::Mat rgb;
        // cv::cvtColor(frameBgr, rgb, cv::COLOR_BGR2RGB);

        // QImage img(rgb.data,
        //            rgb.cols,
        //            rgb.rows,
        //            static_cast<int>(rgb.step),
        //            QImage::Format_RGB888);

        // label->setPixmap(QPixmap::fromImage(img.copy()));

        // display same annotated frame in UI
        //showFrameOnScreen(frameBgr);
    });

    displayTimer->start(33);
    videoThread->start();


    // connect(displayTimer, &QTimer::timeout, this, [this]() {
    //     if (!videoWorker)
    //         return;

    //     cv::Mat frameBgr;
    //     quint64 fid = 0;
    //     qint64 tsMs = 0;

    //     if (!videoWorker->tryGetLatestFrame(frameBgr, fid, tsMs))
    //         return;

    //     if (frameBgr.empty())
    //         return;

    //     // не дублювати кадри
    //     if (fid == lastUdpFrameId)
    //         return;

    //     lastUdpFrameId = fid;

    //     // tracker update + ROI draw on FULL frame
    //     updateTrackerAndOverlay(frameBgr);

    //     // init streamer lazily from actual frame size
    //     if (udpStreamer && !udpStreamer->isReady()) {
    //         udpStreamer->init("192.168.144.15",
    //                           5601,
    //                           frameBgr.cols,
    //                           frameBgr.rows,
    //                           30);
    //     }

    //     // send FULL annotated frame to UDP
    //     if (udpStreamer && udpStreamer->isReady()) {
    //         udpStreamer->sendFrame(frameBgr);
    //     }

    //     if (!uiFpsT.isValid())
    //         uiFpsT.start();

    //     uiCnt++;

    //     const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    //     const qint64 latencyMs = (tsMs > 0) ? (nowMs - tsMs) : -1;

    //     if (uiFpsT.elapsed() >= 1000) {
    //         qDebug() << "[UDP send fps]" << uiCnt
    //                  << "lat(ms)=" << latencyMs
    //                  << "fid=" << fid;

    //         uiCnt = 0;
    //         uiFpsT.restart();
    //     }

    //     // UI output disabled
    //      showFrameOnScreen(frameBgr);
    // });

    // displayTimer->start(33);
    // videoThread->start();

}

void MainWindow::onVideoStatus(const QString &txt)
{
    qDebug() << "[Video]" << txt;
}

void MainWindow::initCAN()
{
    qDebug() << "[MainWindow] thread =" << QThread::currentThread();

    canBus = new CanBus(this);

    connect(canBus, &CanBus::packetReceived,
            this, &MainWindow::handleCANPacket);

    setupParserThread();
    setupQueueTransfer();

    canBus->startReceiving();
}

void MainWindow::handleCANPacket(const QByteArray &packetData)
{
    try {
        CannelloniFrame frame(packetData);

        QMutexLocker locker(&queueMutex);
        activeRX = 50;

        std::queue<std::vector<uint8_t>> frameQueue = frame.GetMessageQueue();

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
    parserThread = new QThread(this);

    parserWorker->moveToThread(parserThread);

    connect(parserThread, &QThread::started,
            parserWorker, &CANParserWorker::process);

    // Тут підключаємо отримання точки по КАН і запуск трекінга
    // Оскільки parserWorker у своєму потоці, а MainWindow в іншому, це має бути queued
    // connect(parserWorker, &CANParserWorker::capturePointReceived,
    //         this, &MainWindow::onCapturePointReceived,
    //         Qt::QueuedConnection);

    connect(parserWorker, &CANParserWorker::capturePointNormalizedReceived,
            this, &MainWindow::onCapturePointNormalizedReceived,
            Qt::QueuedConnection);

    connect(parserWorker, &CANParserWorker::stopTrackingReceived,
            this, &MainWindow::onStopTrackingReceived,
            Qt::QueuedConnection);

    connect(parserThread, &QThread::finished,
            parserWorker, &QObject::deleteLater);

    parserThread->start();
}

void MainWindow::setupQueueTransfer()
{
    QTimer *queueTransferTimer = new QTimer(this);

    connect(queueTransferTimer, &QTimer::timeout,
            this, &MainWindow::transferQueue);

    queueTransferTimer->start(10);
}

void MainWindow::transferQueue()
{
    QMutexLocker locker(&queueMutex);

    std::vector<uint8_t> msg;
    const int maxMsgs = 100;

    for (int i = 0; i < maxMsgs && localMessageQueue.pop(msg); ++i) {
        parserWorker->enqueueMessage(msg);
    }
}


void MainWindow::onStopTrackingReceived()
{
    qDebug() << "[MainWindow] stop tracking received";
    resetTracking();
}

// Скидання трекінга по команді по КАН
void MainWindow::resetTracking()
{
    tracker.release();
    trackingActive = false;
    trackingROI = cv::Rect();

    integralYaw = 0.0f;
    integralPitch = 0.0f;
    previousErrorYaw = 0.0f;
    previousErrorPitch = 0.0f;

    qDebug() << "[TRACKING] reset";
}

// Почати трекінг по нормалізованим координатам
void MainWindow::startTrackingNormalized(float nx, float ny)
{
    cv::Mat frame;
    {
        QMutexLocker locker(&frameMutex);
        if (lastFrame.empty()) {
            qDebug() << "[TRACKING] no frame available for normalized start";
            return;
        }
        frame = lastFrame.clone();
    }

    nx = std::clamp(nx, 0.0f, 1.0f);
    ny = std::clamp(ny, 0.0f, 1.0f);

    const int frameW = frame.cols;
    const int frameH = frame.rows;

    const int x = std::clamp(static_cast<int>(nx * float(frameW - 1)), 0, frameW - 1);
    const int y = std::clamp(static_cast<int>(ny * float(frameH - 1)), 0, frameH - 1);

    qDebug() << "[TRACKING] normalized -> pixel:"
             << "nx =" << nx
             << "ny =" << ny
             << "x =" << x
             << "y =" << y
             << "frame =" << frameW << "x" << frameH;

    startTrackingAtPoint(x, y);
}
