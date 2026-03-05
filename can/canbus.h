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

private slots:
   void readPendingDatagrams();

private:
    QUdpSocket *udpSocket = nullptr;
    quint16 m_port = 14500;
};

#endif // CANBUS_H
