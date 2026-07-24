// canparserworker.h
#pragma once

#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QThread>
#include <vector>



class CANParserWorker : public QObject
{
    Q_OBJECT

public:
    explicit CANParserWorker(QObject *parent = nullptr);

    void enqueueMessage(const std::vector<uint8_t> &message);

signals:
    void messageParsed(/*можна передати структуру*/);
    void parseError(const QString &error);
    void capturePointReceived(uint16_t x, uint16_t y);
    void capturePointNormalizedReceived(float nx, float ny);
    void stopTrackingReceived();
    void cameraFovReceived(float hDeg, float vDeg);
    void trackingParamsReceived(uint16_t roiSize);

public slots:
    void process();

private:
    QQueue<std::vector<uint8_t>> queue;
    QMutex mutex;
    bool running = true;
};
