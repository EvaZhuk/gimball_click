#pragma once

#include <QObject>
#include <QImage>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>

class VideoWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoWorker(QObject *parent = nullptr);
    ~VideoWorker();

public slots:
    void start();          // виклик після запуску thread
    void stop();           // коректна зупинка
    void setRtspUrl(const QString &url);

signals:
    void frameReady(const QImage &img);
    void status(const QString &text);

private:
    bool openStream();
    QImage matToQImageBgr(const cv::Mat &bgr);

private:
    QString m_url;
    cv::VideoCapture m_cap;
    std::atomic_bool m_running{false};
    QElapsedTimer m_reopenTimer;
};
