#include "videoworker.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThread>

VideoWorker::VideoWorker(QObject *parent)
    : QObject(parent)
{
    m_reopenTimer.start();
}

VideoWorker::~VideoWorker()
{
    stop();
    closeSource();
}

void VideoWorker::setSource(const QString &source)
{
    m_source = source.trimmed();
    m_sourceType = detectSourceType(m_source);
}

VideoWorker::SourceType VideoWorker::detectSourceType(const QString &src) const
{
    if (src.isEmpty())
        return SourceType::Unknown;

    const QString s = src.trimmed();

    // /dev/video0, /dev/video7 ...
    static const QRegularExpression reDevVideo("^/dev/video\\d+$");
    if (reDevVideo.match(s).hasMatch())
        return SourceType::CameraDevice;

    // RTSP
    if (s.startsWith("rtsp://", Qt::CaseInsensitive))
        return SourceType::Rtsp;

    // HTTP / HTTPS / UDP / RTP etc. можна розширити при потребі
    if (s.startsWith("http://", Qt::CaseInsensitive) ||
        s.startsWith("https://", Qt::CaseInsensitive))
        return SourceType::Http;

    // GStreamer pipeline:
    // якщо є " ! " або appsink, дуже ймовірно це pipeline
    if (s.contains(" ! ") || s.contains("appsink", Qt::CaseInsensitive))
        return SourceType::GStreamerPipeline;

    // Якщо шлях існує як файл — вважаємо файлом
    QFileInfo fi(s);
    if (fi.exists() && fi.isFile())
        return SourceType::File;

    return SourceType::Unknown;
}

bool VideoWorker::isLiveSource(SourceType t) const
{
    switch (t) {
    case SourceType::CameraDevice:
    case SourceType::Rtsp:
    case SourceType::Http:
    case SourceType::GStreamerPipeline:
        return true;
    case SourceType::File:
    case SourceType::Unknown:
    default:
        return false;
    }
}

void VideoWorker::closeSource()
{
    if (m_cap.isOpened())
        m_cap.release();

    m_frameWidth = 0;
    m_frameHeight = 0;
    m_fps = 0.0;
}

bool VideoWorker::openSource()
{
    closeSource();

    if (m_source.isEmpty()) {
        emit status("Video source is empty");
        return false;
    }

    bool ok = false;
    const std::string src = m_source.toStdString();

    switch (m_sourceType) {
    case SourceType::CameraDevice:
        // /dev/videoX -> CAP_V4L2
        emit status("Opening V4L2 device: " + m_source);
        ok = m_cap.open(src, cv::CAP_V4L2);

        if (ok) {
            // необов'язково, але часто корисно
            m_cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
            // за потреби можна задати формат:
            // m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
            // m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
            // m_cap.set(cv::CAP_PROP_FPS, 30);
        }
        break;

    case SourceType::Rtsp:
        emit status("Opening RTSP stream via FFMPEG: " + m_source);
        ok = m_cap.open(src, cv::CAP_FFMPEG);

        // fallback
        if (!ok) {
            emit status("FFMPEG failed, trying GStreamer for RTSP...");
            ok = m_cap.open(src, cv::CAP_GSTREAMER);
        }

        if (ok)
            m_cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
        break;

    case SourceType::Http:
        emit status("Opening HTTP stream: " + m_source);
        ok = m_cap.open(src, cv::CAP_FFMPEG);

        if (!ok) {
            emit status("FFMPEG failed, trying CAP_ANY...");
            ok = m_cap.open(src, cv::CAP_ANY);
        }
        break;

    case SourceType::GStreamerPipeline:
        emit status("Opening GStreamer pipeline");
        ok = m_cap.open(src, cv::CAP_GSTREAMER);
        break;

    case SourceType::File:
        emit status("Opening file: " + m_source);

        // для файлу CAP_ANY найлегше
        ok = m_cap.open(src, cv::CAP_ANY);

        // fallback
        if (!ok)
            ok = m_cap.open(src, cv::CAP_FFMPEG);
        break;

    case SourceType::Unknown:
    default:
        emit status("Unknown source type, trying CAP_ANY: " + m_source);
        ok = m_cap.open(src, cv::CAP_ANY);

        if (!ok)
            ok = m_cap.open(src, cv::CAP_FFMPEG);

        if (!ok)
            ok = m_cap.open(src, cv::CAP_GSTREAMER);

        if (!ok && m_source.startsWith("/dev/video"))
            ok = m_cap.open(src, cv::CAP_V4L2);
        break;
    }

    if (!ok) {
        emit status("Failed to open video source: " + m_source);
        return false;
    }

    // Дізнаємося і виводимо розмір захопленого фрейма

    m_frameWidth  = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_WIDTH));
    m_frameHeight = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_fps         = m_cap.get(cv::CAP_PROP_FPS);

    qDebug() << "[Video] opened:"
             << "width =" << m_frameWidth
             << "height =" << m_frameHeight
             << "fps =" << m_fps;


    emit status("Video source opened");


    return true;
}

bool VideoWorker::readOneFrame(cv::Mat &frame)
{
    if (!m_cap.isOpened())
        return false;

    if (!m_cap.read(frame))
        return false;

    if (frame.empty())
        return false;

    return true;
}

bool VideoWorker::tryGetLatestFrame(cv::Mat &outBgr, quint64 &outId, qint64 &outTsMs)
{
    QMutexLocker lk(&m_frameMtx);
    if (m_latestBgr.empty())
        return false;

    outBgr = m_latestBgr.clone();
    outId = m_latestId;
    outTsMs = m_latestTsMs;
    return true;
}

void VideoWorker::start()
{
    if (m_running)
        return;

    m_running = true;
    m_reopenTimer.restart();

    if (!openSource()) {
        emit status("Initial open failed");
    }

    int filePeriodMs = 0;
    if (m_sourceType == SourceType::File && m_cap.isOpened()) {
        double fps = m_cap.get(cv::CAP_PROP_FPS);
        if (fps < 1.0 || fps > 240.0)
            fps = 25.0;
        filePeriodMs = static_cast<int>(1000.0 / fps);
    }

    cv::Mat frame;

    QElapsedTimer capFpsT;
    capFpsT.start();
    int capCnt = 0;

    while (m_running) {

        if (!readOneFrame(frame)) {

            // файл -> перемотати
            if (m_sourceType == SourceType::File) {
                if (m_cap.isOpened()) {
                    m_cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                    QThread::msleep(5);
                    continue;
                } else {
                    if (m_reopenTimer.elapsed() > 1000) {
                        emit status("File read failed. Reopening...");
                        openSource();
                        m_reopenTimer.restart();
                    }
                    QThread::msleep(20);
                    continue;
                }
            }

            // live source -> reopen
            if (isLiveSource(m_sourceType)) {
                if (m_reopenTimer.elapsed() > 1000) {
                    emit status("Read failed. Reopening live source...");
                    openSource();
                    m_reopenTimer.restart();
                }
                QThread::msleep(10);
                continue;
            }

            // unknown
            QThread::msleep(20);
            continue;
        }

        capCnt++;
        if (capFpsT.elapsed() >= 1000) {
            qDebug() << "[VW capture fps]" << capCnt;
            capCnt = 0;
            capFpsT.restart();
        }

        {
            QMutexLocker lk(&m_frameMtx);
            m_latestBgr = frame.clone();
            ++m_latestId;
            m_latestTsMs = QDateTime::currentMSecsSinceEpoch();
        }

        // для файлу штучно тримаємо темп
        if (m_sourceType == SourceType::File && filePeriodMs > 0) {
            QThread::msleep(filePeriodMs);
        }
    }

    closeSource();
    emit status("VideoWorker stopped");
}

void VideoWorker::stop()
{
    m_running = false;
}

int VideoWorker::frameWidth() const
{
    return m_frameWidth;
}

int VideoWorker::frameHeight() const
{
    return m_frameHeight;
}

double VideoWorker::fps() const
{
    return m_fps;
}
