#include "udpstreamer.h"

UdpStreamer::UdpStreamer(QObject *parent) : QObject(parent), isInitialized(false)
{
}

UdpStreamer::~UdpStreamer()
{
    stop();
}

bool UdpStreamer::init(const QString &targetIp, int port, int width, int height, int fps)
{
    if (isInitialized) {
        stop();
    }

    streamWidth = width;
    streamHeight = height;

    // GStreamer Pipeline Description:
    // 1. appsrc: OpenCV pushes frames here
    // 2. videoconvert: Ensures color compatibility
    // 3. x264enc: Compresses video (zerolatency for real-time control)
    // 4. rtph264pay: Packages h264 into RTP packets
    // 5. udpsink: Sends packets to target
    QString pipeline = QString(
                           "appsrc ! "
                           "videoconvert ! "
                           "x264enc tune=zerolatency bitrate=2048 speed-preset=ultrafast ! "
                           "rtph264pay config-interval=1 pt=96 ! "
                           "udpsink host=%1 port=%2"
                           ).arg(targetIp).arg(port);

    // Open the writer using the GStreamer backend
    writer.open(pipeline.toStdString(), cv::CAP_GSTREAMER, 0, fps, cv::Size(width, height), true);

    if (!writer.isOpened()) {
        qDebug() << "[UdpStreamer] Failed to open GStreamer pipeline!";
        qDebug() << "[UdpStreamer] Ensure OpenCV is compiled with GStreamer support.";
        isInitialized = false;
        return false;
    }

    qDebug() << "[UdpStreamer] Streaming to" << targetIp << ":" << port;
    isInitialized = true;
    return true;
}

void UdpStreamer::sendFrame(const cv::Mat &frame)
{
    if (!isInitialized || frame.empty()) return;

    // If the frame size changed unexpectedly, we might need to resize or handle error
    if (frame.cols != streamWidth || frame.rows != streamHeight) {
        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(streamWidth, streamHeight));
        writer.write(resized);
    } else {
        writer.write(frame);
    }
}

void UdpStreamer::stop()
{
    if (writer.isOpened()) {
        writer.release();
    }
    isInitialized = false;
}
