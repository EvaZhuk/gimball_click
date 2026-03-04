#ifndef CANBUS_H
#define CANBUS_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QUdpSocket>

class CanBus : public QObject
{
    Q_OBJECT

public:
    explicit CanBus(QObject *parent = nullptr);
    ~CanBus();

    void startReceiving();
    void stopReceiving();

    // Зробити метод публічним
    QString toHexString(const QByteArray &data); // Перетворення байтового масиву в формат hex

signals:
    void packetReceived(const QByteArray &data); // Сигнал для повідомлення про отриманий пакет

private:
    QUdpSocket *udpSocket;
    void readPendingDatagrams();
};

#endif // CANBUS_H
