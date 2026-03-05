#include "videoworker.h"
#include "qdebug.h"
#include <QThread>

VideoWorker::VideoWorker(QObject *parent) : QObject(parent)
{
    m_reopenTimer.start();
}

VideoWorker::~VideoWorker()
{
    stop();
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

QImage VideoWorker::matToQImageBgr(const cv::Mat &bgr)
{
    // bgr: CV_8UC3
    QImage img(bgr.data, bgr.cols, bgr.rows, (int)bgr.step, QImage::Format_BGR888);
    return img.copy(); // важливо: копія, бо Mat буде перезаписано
}

void VideoWorker::start()
{
    m_running = true;

    openStream();

    auto updateFramePeriod = [&]() -> int {
        double fps = m_cap.get(cv::CAP_PROP_FPS);

        // Для файлу FPS зазвичай є, для RTSP може бути 0/сміття
        if (fps < 5.0 || fps > 120.0) {
            // fallback: RTSP ~25/30, файл теж можна 25
            fps = 30.0;
        }
        return int(1000.0 / fps);
    };

    int framePeriodMs = updateFramePeriod();

    cv::Mat frame;

    QElapsedTimer fpsT;
    fpsT.start();
    int fpsCnt = 0;

    while (m_running) {

        if (!m_cap.isOpened() || !m_cap.read(frame) || frame.empty()) {



            // якщо файл — перемотати на початок
            if (m_url.endsWith(".mp4") || m_url.endsWith(".avi") || m_url.endsWith(".mkv")) {
                m_cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                QThread::msleep(5);
                continue;
            }

            // якщо RTSP — reopen не частіше 1 раз/сек
            if (m_reopenTimer.elapsed() > 1000) {
                emit status("Read failed. Reopening RTSP...");
                openStream();
                framePeriodMs = updateFramePeriod();
                m_reopenTimer.restart();
            }

            QThread::msleep(10);
            continue;
        }

        // лічильник fps (debug)
        fpsCnt++;
        if (fpsT.elapsed() >= 1000) {
            qDebug() << "[VW emit fps]" << fpsCnt << "period(ms)=" << framePeriodMs;
            fpsCnt = 0;
            fpsT.restart();
        }


        bool isFile =
            m_url.endsWith(".mp4") ||
            m_url.endsWith(".avi") ||
            m_url.endsWith(".mkv");

        emit frameReady(matToQImageBgr(frame));

        if (isFile && framePeriodMs > 0)
            QThread::msleep(framePeriodMs);
    }

    if (m_cap.isOpened()) m_cap.release();
    emit status("VideoWorker stopped");
}

void VideoWorker::stop()
{
    m_running = false;
}
