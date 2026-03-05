#pragma once

#include <QObject>
#include <QImage>
#include <QElapsedTimer>
#include <QMutex>
#include <opencv2/opencv.hpp>

class VideoWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoWorker(QObject *parent = nullptr);
    ~VideoWorker();
    bool tryGetLatestFrame(cv::Mat &outBgr, quint64 &outId, qint64 &outTsMs);
    void setRtspUrl(const QString &url);

public slots:
    void start();          // виклик після запуску thread
    void stop();           // коректна зупинка


signals:
    void status(const QString &text);

private:
    bool openStream();

private:
    QString m_url;
    cv::VideoCapture m_cap;
    std::atomic_bool m_running{false};
    QElapsedTimer m_reopenTimer;

    //latest frame
    QMutex m_frameMtx;
    cv::Mat m_latestBgr;
    qint64 m_latestId = 0;
    qint64  m_latestTsMs = 0;   // timestamp кадру (ms)

};
