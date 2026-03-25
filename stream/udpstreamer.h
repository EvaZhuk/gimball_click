#ifndef UDPSTREAMER_H
#define UDPSTREAMER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <opencv2/opencv.hpp>

class UdpStreamer : public QObject
{
    Q_OBJECT
public:
    explicit UdpStreamer(QObject *parent = nullptr);
    ~UdpStreamer();

    bool init(const QString &targetIp, int port, int width, int height, int fps);
    void sendFrame(const cv::Mat &frame);
    void stop();

    bool isReady() const;

private:
    cv::VideoWriter writer;
    bool isInitialized = false;
    int streamWidth = 0;
    int streamHeight = 0;
};

#endif // UDPSTREAMER_H
