#include "videoworker.h"
#include "qdebug.h"
#include <QThread>
#include <QDateTime>

VideoWorker::VideoWorker(QObject *parent) : QObject(parent)
{
    m_reopenTimer.start();
}

VideoWorker::~VideoWorker()
{
    stop();
    if (m_cap.isOpened()) m_cap.release();
}

void VideoWorker::setRtspUrl(const QString &url)
{
    m_url = url;
}

bool VideoWorker::openStream()
{
    if (m_url.isEmpty()) {
        emit status("RTSP url is empty");
        return false;
    }

    if (m_cap.isOpened()) {
        m_cap.release();
    }

    // CAP_FFMPEG під Linux часто ок. Якщо треба — прибери другий аргумент.
    bool ok = m_cap.open(m_url.toStdString(), cv::CAP_FFMPEG);
    if (!ok) {
        emit status("Failed to open RTSP stream");
        return false;
    }
    emit status("Video source opened");
    return true;
}

bool VideoWorker::tryGetLatestFrame(cv::Mat &outBgr, quint64 &outId, qint64 &outTsMs)
{
    QMutexLocker lk(&m_frameMtx);
    if (m_latestBgr.empty()) return false;

    outBgr = m_latestBgr.clone();
    outId  = m_latestId;
    outTsMs = m_latestTsMs;
    return true;
}


void VideoWorker::start()
{
    m_running = true;

    openStream();

    //перевірка чи це файл, чи потік
    const bool isFile =
        m_url.endsWith(".mp4", Qt::CaseInsensitive) ||
        m_url.endsWith(".avi", Qt::CaseInsensitive) ||
        m_url.endsWith(".mkv", Qt::CaseInsensitive);


    int filePeriodMs = 0;
    if (isFile) {
        double fps = m_cap.get(cv::CAP_PROP_FPS);
        if (fps < 5.0 || fps > 120.0) fps = 25.0;   // fallback
        filePeriodMs = int(1000.0 / fps);
    }

    cv::Mat frame;

    // fps контроль
    QElapsedTimer capFpsT;
    capFpsT.start();
    int capCnt = 0;

    while (m_running) {

        if (!m_cap.isOpened() || !m_cap.read(frame) || frame.empty()) {
            // якщо файл — перемотати на початок
            if (isFile) {
                m_cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                QThread::msleep(5);
                continue;
            }

            // якщо RTSP — reopen не частіше 1 раз/сек
            if (m_reopenTimer.elapsed() > 1000) {
                emit status("Read failed. Reopening RTSP...");
                openStream();
                m_reopenTimer.restart();
            }

            QThread::msleep(10);
            continue;
        }

        // лічильник fps (debug)раз/сек
        capCnt++;
        if (capFpsT.elapsed() >= 1000) {
            qDebug() << "[VW capture fps]" << capCnt;
            capCnt = 0;
            capFpsT.restart();
        }

        // update latest frame + id + timestamp
        {
            QMutexLocker lk(&m_frameMtx);
            m_latestBgr = frame.clone();
            m_latestId++;
            m_latestTsMs = QDateTime::currentMSecsSinceEpoch();


        }

        //затримка для файлу - бо дуже швидко відображається
        if (isFile && filePeriodMs > 0)
            QThread::msleep(filePeriodMs);
    }

    if (m_cap.isOpened()) m_cap.release();
    emit status("VideoWorker stopped");
}


void VideoWorker::stop()
{
    m_running = false;
}
