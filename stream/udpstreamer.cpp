#include "udpstreamer.h"

UdpStreamer::UdpStreamer(QObject *parent)
    : QObject(parent)
{
}

bool UdpStreamer::isReady() const
{
    return isInitialized;
}

UdpStreamer::~UdpStreamer()
{
    stop();
}

bool UdpStreamer::init(const QString &targetIp, int port, int width, int height, int fps)
{
    stop();

    streamWidth = width;
    streamHeight = height;

    // IMPORTANT:
    // receiver expects:
    // udpsrc ! tsparse ! tsdemux ! h264parse ! avdec_h264 ! ...
    //
    // so sender must send MPEG-TS with H264 inside, NOT RTP
    QString pipeline = QString(
                           "appsrc ! "
                           "queue ! "
                           "videoconvert ! "
                           "video/x-raw,format=I420,width=%1,height=%2,framerate=%3/1 ! "
                           "x264enc tune=zerolatency speed-preset=ultrafast bitrate=2048 key-int-max=%3 ! "
                           "h264parse ! "
                           "mpegtsmux ! "
                           "udpsink host=%4 port=%5 sync=false async=false"
                           ).arg(width).arg(height).arg(fps).arg(targetIp).arg(port);

    qDebug() << "[UdpStreamer] open pipeline =" << pipeline;

    writer.open(pipeline.toStdString(),
                cv::CAP_GSTREAMER,
                0,
                fps,
                cv::Size(width, height),
                true);

    if (!writer.isOpened()) {
        qDebug() << "[UdpStreamer] Failed to open GStreamer MPEG-TS pipeline";
        isInitialized = false;
        return false;
    }

    isInitialized = true;
    qDebug() << "[UdpStreamer] Streaming to" << targetIp << ":" << port
             << "size=" << width << "x" << height << "fps=" << fps;
    return true;
}

void UdpStreamer::sendFrame(const cv::Mat &frame)
{
    if (!isInitialized || frame.empty())
        return;

    if (frame.cols == streamWidth && frame.rows == streamHeight) {
        writer.write(frame);
        return;
    }

    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(streamWidth, streamHeight));
    writer.write(resized);
}

void UdpStreamer::stop()
{
    if (writer.isOpened())
        writer.release();

    isInitialized = false;
}
