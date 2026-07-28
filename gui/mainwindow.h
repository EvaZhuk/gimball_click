#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QSize>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QThread>

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

#include "can/message/canmessagegeneric.h"
#include "can/parser/canparserworker.h"
#include "can/utils/CircularBuffer.h"
#include "can/canbus.h"
#include "clickablelabel.h"

#include "stream/udpstreamer.h"
#include "video/videoworker.h"
#include "tracking/trackingdeviationcalculator.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLabelClicked(QPoint pos);
    void onVideoStatus(const QString &txt);

    void onCapturePointReceived(quint16 x, quint16 y);
    void onCapturePointNormalizedReceived(float nx, float ny);
    void onStopTrackingReceived();
    void onCameraFovReceived(float hDeg, float vDeg);
    void onTrackingParamsReceived(uint16_t roiSize);

private:

    void initUI();
    void initVideoThread();
    void initCAN();

    void setupParserThread();
    void setupQueueTransfer();
    void handleCANPacket(const QByteArray &packetData);
    void transferQueue();

    // tracking helpers
    bool mapLabelPointToFrame(const QPoint &pos, const cv::Mat &frame, cv::Point &framePt);
    void updateTrackerAndOverlay(cv::Mat &frame);
    void drawTrackingOverlay(cv::Mat &frame, bool ok);
    void showFrameOnScreen(const cv::Mat &frameBgr);
private:
    ClickableLabel *label = nullptr;
    QTimer *displayTimer = nullptr;

    cv::Mat lastFrame;
    QMutex frameMutex;

    QThread *videoThread = nullptr;
    VideoWorker *videoWorker = nullptr;

    UdpStreamer *udpStreamer = nullptr;

    // UI control
    QElapsedTimer uiFpsT;
    int uiCnt = 0;
    quint64 lastDrawId = 0;

    // CAN
    CanBus *canBus = nullptr;
    int activeRX = 0;
    CircularBuffer<std::vector<uint8_t>> localMessageQueue;
    QMutex queueMutex;

    CANParserWorker *parserWorker = nullptr;
    QThread *parserThread = nullptr;

    // tracker
    cv::Ptr<cv::Tracker> tracker;
    cv::Rect trackingROI;
    bool trackingActive = false;

    // PID vars
    float Kp_yaw = 0.0f, Ki_yaw = 0.0f, Kd_yaw = 0.0f;
    float Kp_pitch = 0.0f, Ki_pitch = 0.0f, Kd_pitch = 0.0f;

    float integralYaw = 0.0f;
    float integralPitch = 0.0f;
    float previousErrorYaw = 0.0f;
    float previousErrorPitch = 0.0f;

    const float FOV_HORIZONTAL_DEG = 107.8f;
    const float FOV_VERTICAL_DEG   = 74.6f;

    CameraFov cameraFov;
    TrackingParams trackingParams;

    void startTrackingAtPoint(int xCenter, int yCenter);
    void startTrackingNormalized(float nx, float ny);
    void resetTracking();

    quint64 lastUdpFrameId = 0;
    TrackingDeviationCalculator deviationCalculator;
    void resizeActiveTrackingRoi(uint16_t roiSize);
};

#endif // MAINWINDOW_H
