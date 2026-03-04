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

public slots:
    void process();

private:
    QQueue<std::vector<uint8_t>> queue;
    QMutex mutex;
    bool running = true;
};
