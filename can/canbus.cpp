#include "canbus.h"
#include <QDebug>
#include <QHostAddress>
#include <QUdpSocket>

CanBus::CanBus(QObject *parent)
    : QObject(parent)
    , udpSocket(new QUdpSocket(this))
{
    // Тут можна налаштувати сокет для отримання даних
}

CanBus::~CanBus()
{
    stopReceiving();
}

void CanBus::startReceiving()
{
    // Відкриваємо порт для отримання пакетів
    if (udpSocket->bind(QHostAddress::Any, 14500)) { // Вказати правильний порт
        connect(udpSocket, &QUdpSocket::readyRead, this, &CanBus::readPendingDatagrams);
    }
}

void CanBus::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        // Тут обробка отриманого пакету
        emit packetReceived(datagram); // Сигнал, який повідомляє про отриманий пакет
    }
}

void CanBus::stopReceiving()
{
    // Закриваємо сокет
    udpSocket->close();
}

QString CanBus::toHexString(const QByteArray &data)
{
    QString hexString;
    for (int i = 0; i < data.size(); ++i) {
        hexString.append(QString::asprintf("%02X ", static_cast<unsigned char>(data[i])));
    }
    return hexString.trimmed();
}
