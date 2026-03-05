#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QSize>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

#include "can/parser/canparserworker.h"
#include "can/utils/CircularBuffer.h"
#include "can/canbus.h"
#include "clickablelabel.h"

#include "stream/udpstreamer.h"
#include "video/videoworker.h"
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void onLabelClicked(QPoint pos);
    void onFrameReady(const QImage &img);
    void onVideoStatus(const QString &txt);

private:
    ClickableLabel *label;
    QTimer *timer;
    cv::VideoCapture cap;
    UdpStreamer udpStreamer;
    QSize videoSize;
    CanBus *canBus;
    // Змінні для PID контролера
    float Kp_yaw, Ki_yaw, Kd_yaw;
    float Kp_pitch, Ki_pitch, Kd_pitch;

    float integralYaw;
    float integralPitch;
    float previousErrorYaw;
    float previousErrorPitch;

    // Поле зору камери (для об'єктива 4.3mm згідно з мануалом Siyi A8 Mini v1.6)
    // Якщо у вас інший об'єктив, змініть ці значення
    const float FOV_HORIZONTAL_DEG = 107.8f;
    const float FOV_VERTICAL_DEG = 74.6f;

    cv::Ptr<cv::TrackerCSRT> tracker;
    cv::Rect2d trackingROI;
    bool trackingActive = false;
    void drawFPS(cv::Mat frame);

    int activeRX = 0;
    CircularBuffer<std::vector<uint8_t>> localMessageQueue;
    QMutex queueMutex;
    QMutex frameMutex;

    CANParserWorker *parserWorker;
    QThread *parserThread;

    QThread *videoThread = nullptr;
    VideoWorker *videoWorker = nullptr;

    cv::Mat lastFrame;

    void initUI();
    void initVideo();
    void initVideoThread();
    void initCAN();

    void setupParserThread();
    void setupQueueTransfer();

    void handleCANPacket(const QByteArray &packetData);
    void transferQueue();

};

#endif // MAINWINDOW_H
