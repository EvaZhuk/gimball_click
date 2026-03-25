#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QMutex>
#include <QString>
#include <atomic>
#include <opencv2/opencv.hpp>

class VideoWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoWorker(QObject *parent = nullptr);
    ~VideoWorker();

    void setSource(const QString &source);
    bool tryGetLatestFrame(cv::Mat &outBgr, quint64 &outId, qint64 &outTsMs);

public slots:
    void start();
    void stop();

signals:
    void status(const QString &text);

private:
    enum class SourceType {
        Unknown,
        File,
        CameraDevice,       // /dev/video0
        Rtsp,
        Http,
        GStreamerPipeline
    };

private:
    SourceType detectSourceType(const QString &src) const;
    bool openSource();
    void closeSource();
    bool readOneFrame(cv::Mat &frame);
    bool isLiveSource(SourceType t) const;

private:
    QString m_source;
    SourceType m_sourceType = SourceType::Unknown;

    cv::VideoCapture m_cap;
    std::atomic_bool m_running{false};

    QElapsedTimer m_reopenTimer;

    QMutex m_frameMtx;
    cv::Mat m_latestBgr;
    quint64 m_latestId = 0;
    qint64  m_latestTsMs = 0;
};
