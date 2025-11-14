#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QSize>
#include <QTimer>
#include <QElapsedTimer>

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

#include "clickablelabel.h"
#include "siyisender.h"

#include "udpstreamer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void onLabelClicked(QPoint pos);

private:
    ClickableLabel *label;
    QTimer *timer;
    cv::VideoCapture cap;
    SiyiSender siyi;
    UdpStreamer udpStreamer;
    QSize videoSize;

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

};

#endif // MAINWINDOW_H
