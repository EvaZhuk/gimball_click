#ifndef UDPSTREAMER_H
#define UDPSTREAMER_H

#include <QObject>
#include <QString>
#include <opencv2/opencv.hpp>
#include <QDebug>

class UdpStreamer : public QObject
{
    Q_OBJECT
public:
    explicit UdpStreamer(QObject *parent = nullptr);
    ~UdpStreamer();

    // Initialize the GStreamer pipeline
    // targetIp: The IP to send to (e.g., "127.0.0.1" or a remote IP)
    // port: The UDP port (e.g., 5600)
    bool init(const QString &targetIp, int port, int width, int height, int fps);

    // Send the processed frame
    void sendFrame(const cv::Mat &frame);

    void stop();

private:
    cv::VideoWriter writer;
    bool isInitialized;
    int streamWidth;
    int streamHeight;
};

#endif // UDPSTREAMER_H
