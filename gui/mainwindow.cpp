#include "gui/mainwindow.h"
#include "can/parser/canparserworker.h"
#include "can/transport/CannelloniFrame.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <QMutexLocker>
#include <algorithm> // Для std::clamp
#include <can/canbus.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    label(new ClickableLabel(this)),
    timer(new QTimer(this)),
    localMessageQueue{1000}
{
    initUI();
    initVideoThread();
    initCAN();

    //timer->start(30); // 30 мс інтервал оновлення (приблизно 33.3 кадри/сек)
}

MainWindow::~MainWindow() {
    cap.release();
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

void MainWindow::updateFrame() {
    cv::Mat frame;
    if (!cap.read(frame)) {
        qDebug() << "Failed to read frame from RTSP stream. Attempting to reopen...";
        cap.release();
        cap.open("rtsp://192.168.144.25:8554/main.264", cv::CAP_FFMPEG);
        if (!cap.isOpened()) {
            qDebug() << "Still failed to open RTSP stream.";
        }
        return;
    }

    if (trackingActive && tracker) {
        cv::Rect tempIntRect = trackingROI;
        bool ok = tracker->update(frame, tempIntRect);

        if (ok) {
            trackingROI = tempIntRect;
            cv::rectangle(frame, trackingROI, cv::Scalar(0, 255, 0), 2); // Зелений прямокутник

            float cx = trackingROI.x + trackingROI.width / 2.0f;
            float cy = trackingROI.y + trackingROI.height / 2.0f;

            // Вимірюємо помилку в пікселях від центру кадру
            float pixel_error_x = cx - frame.cols / 2.0f;
            float pixel_error_y = frame.rows / 2.0f - cy; // Y вісь інвертована для pitch (позитивний вгору)

            // Перетворюємо піксельну помилку в кутову помилку (градуси)
            float angle_per_pixel_yaw = FOV_HORIZONTAL_DEG / frame.cols;
            float angle_per_pixel_pitch = FOV_VERTICAL_DEG / frame.rows;

            float errorYaw = pixel_error_x * angle_per_pixel_yaw;
            float errorPitch = pixel_error_y * angle_per_pixel_pitch;

            float dt = timer->interval() / 1000.0f; // Час в секундах між оновленнями

            // // PID розрахунки для Yaw (панорама)
            // integralYaw += errorYaw * dt;
            // // Обмеження інтегральної складової для запобігання "wind-up"
            // integralYaw = std::clamp(integralYaw, -100.0f, 100.0f); // Приклад меж

            // float derivativeYaw = (errorYaw - previousErrorYaw) / dt;
            // previousErrorYaw = errorYaw;

            // float targetSpeedYaw = Kp_yaw * errorYaw + Ki_yaw * integralYaw + Kd_yaw * derivativeYaw;

            // // PID розрахунки для Pitch (нахил)
            // integralPitch += errorPitch * dt;
            // // Обмеження інтегральної складової
            // integralPitch = std::clamp(integralPitch, -100.0f, 100.0f); // Приклад меж

            // float derivativePitch = (errorPitch - previousErrorPitch) / dt;
            // previousErrorPitch = errorPitch;

            // float targetSpeedPitch = Kp_pitch * errorPitch + Ki_pitch * integralPitch + Kd_pitch * derivativePitch;

            // // Обмеження розрахованих швидкостей PID-контролером до розумних меж (наприклад, 30 deg/sec)
            // // Ці значення будуть масштабуватися в siyisender.cpp до -100..100
            // targetSpeedYaw = std::clamp(targetSpeedYaw, -30.0f, 30.0f);
            // targetSpeedPitch = std::clamp(targetSpeedPitch, -30.0f, 30.0f);

            // // Відправка команд швидкості до гімбала Siyi
            // siyi.sendSpeeds(targetSpeedYaw, targetSpeedPitch, 0.0f); // Roll speed зазвичай 0 для трекінгу

            // qDebug() << "[PID Output] Yaw Speed:" << targetSpeedYaw << "Pitch Speed:" << targetSpeedPitch;

        } else {
            qDebug() << "TRACKER LOST - Resetting PID and stopping gimbal.";
            trackingActive = false;
            // Скидання інтегральної складової та попередніх помилок при втраті трекера
            /*integralYaw = 0.0f;
            integralPitch = 0.0f;
            previousErrorYaw = 0.0f;
            previousErrorPitch = 0.0f;
            // Відправити нульові швидкості, щоб камера зупинилася
            siyi.sendSpeeds(0.0f, 0.0f, 0.0f);*/
        }
    }

    // Відображення центрального хрестика на кадрі
    int cx_frame = frame.cols / 2;
    int cy_frame = frame.rows / 2;
    int size = 20;
    cv::line(frame, cv::Point(cx_frame - size, cy_frame), cv::Point(cx_frame + size, cy_frame), cv::Scalar(0, 0, 255), 2); // Червоний хрестик
    cv::line(frame, cv::Point(cx_frame, cy_frame - size), cv::Point(cx_frame, cy_frame + size), cv::Scalar(0, 0, 255), 2);

    drawFPS(frame);
    //udpStreamer.sendFrame(frame);

    // Відображення кадру на QLabel
    cv::resize(frame, frame, cv::Size(label->width(), label->height()));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(img));
}



void MainWindow::onLabelClicked(QPoint pos) {
    qDebug() << "Clicked on screen:" << pos;

    if (lastFrame.empty()) {
        qDebug() << "No frame available for tracker init";
        return;
    }

    QMutexLocker locker(&frameMutex);
    cv::Mat initFrame = lastFrame.clone();

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




void MainWindow::drawFPS(cv::Mat frame)
{
    static int frameCount = 0;
    static QElapsedTimer timer;
    static qint64 lastTime = 0;
    static double fps = 0.0;

    if(!timer.isValid()){
        timer.start();
    }

    frameCount++;
    qint64 elapsed = timer.elapsed();

    if(elapsed - lastTime >= 1000){ //each second
        fps = (frameCount*1000)/(elapsed-lastTime);
        lastTime = elapsed;
        frameCount = 0;
    }

    //draw FP{S at upper left corner
    cv::putText(frame,
                cv::format("FPS: %.1f", fps),
                cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(0,255,0),
                2,
                cv::LINE_AA);
}



void MainWindow::initVideoThread()
{
    videoThread = new QThread(this);
    videoWorker = new VideoWorker();

    videoWorker->moveToThread(videoThread);


    // open video source
    //cap.open("rtsp://192.168.144.25:8554/main.264", cv::CAP_FFMPEG);
    //cap.open("/dev/video7");
    //videoWorker->setRtspUrl("rtsp://192.168.144.25:8554/main.264");
    videoWorker->setRtspUrl("/home/lps/2025-10-14 14-52-14.mp4");

    connect(videoThread, &QThread::started, videoWorker, &VideoWorker::start);
    connect(this, &MainWindow::destroyed, videoWorker, &VideoWorker::stop);

    connect(videoWorker, &VideoWorker::frameReady, this, &MainWindow::onFrameReady, Qt::QueuedConnection);
    connect(videoWorker, &VideoWorker::status, this, &MainWindow::onVideoStatus, Qt::QueuedConnection);

    // cleanup
    connect(videoThread, &QThread::finished, videoWorker, &QObject::deleteLater);

    videoThread->start();



    // cap.open("/home/lps/2025-10-14 14-52-14.mp4");
    // if (!cap.isOpened()) {
    //     qDebug() << "Failed to open RTSP stream. Check camera IP/port or network connection.";
    //     return;
    // }

    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    // videoSize = QSize(1920, 1080);
}

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


void MainWindow::onFrameReady(const QImage &img)
{
    static QElapsedTimer fpsT;
    static int fpsCnt = 0;
    if (!fpsT.isValid()) fpsT.start();

    fpsCnt++;
    if (fpsT.elapsed() >= 1000) {
        qDebug() << "[UI draw fps]" << fpsCnt;
        fpsCnt = 0;
        fpsT.restart();
    }


    // конвертація назад у cv::Mat
    cv::Mat frame(img.height(),
                  img.width(),
                  CV_8UC3,
                  const_cast<uchar*>(img.bits()),
                  img.bytesPerLine());

    QMutexLocker locker(&frameMutex);
    lastFrame = frame.clone();   // зберігаємо копію

    // якщо label — ClickableLabel
    label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::onVideoStatus(const QString &txt)
{
    qDebug() << "[Video]" << txt;
}

void MainWindow::initCAN()
{
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
