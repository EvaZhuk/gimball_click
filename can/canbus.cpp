#include "canbus.h"
#include "qthread.h"
#include <QDebug>
#include <QHostAddress>
#include <QUdpSocket>

CanBus::CanBus(QObject *parent)
    : QObject(parent)
    , udpSocket(new QUdpSocket(this))
{
    // Тут можна налаштувати сокет для отримання даних
    qDebug() << "[CanBus ctor] thread =" << QThread::currentThread();
}

CanBus::~CanBus()
{
    stopReceiving();
}

/*
void CanBus::startReceiving()
{
    // Відкриваємо порт для отримання пакетів
    if (udpSocket->bind(QHostAddress::Any, 14500)) { // Вказати правильний порт
        connect(udpSocket, &QUdpSocket::readyRead, this, &CanBus::readPendingDatagrams);
    }
}*/


void CanBus::startReceiving()
{
    // 1) Переконуємось, що сокет у Qt-потоці
    qDebug() << "[CanBus] thread =" << QThread::currentThread()
             << " objectThread =" << this->thread();

    if (QThread::currentThread() != this->thread()) {
        qWarning() << "[CanBus] startReceiving() called from wrong thread!";
        // Перенаправляємо виклик у правильний потік (event loop потрібен)
        QMetaObject::invokeMethod(this, "startReceiving", Qt::QueuedConnection);
        return;
    }

    if (!udpSocket) {
        udpSocket = new QUdpSocket(this);
    }

    // 2) Підключення readyRead робимо 1 раз
    static bool connected = false;
    if (!connected) {
        connect(udpSocket, &QUdpSocket::readyRead,
                this, &CanBus::readPendingDatagrams);
        connected = true;
    }

    // 3) Якщо вже був bind — закриваємо
    if (udpSocket->state() != QAbstractSocket::UnconnectedState) {
        udpSocket->close();
    }

    // 4) Bind з reuse/share (часто вирішує "port busy" у локальних тестах)
    const bool ok = udpSocket->bind(QHostAddress::AnyIPv4, m_port,
                                    QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!ok) {
        qWarning() << "[CanBus] bind failed port=" << m_port
                   << "error=" << udpSocket->errorString();
        return;
    }

    qDebug() << "[CanBus] UDP bind OK port=" << m_port;
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

// void CanBus::stopReceiving()
// {
//     // Закриваємо сокет
//     udpSocket->close();
// }

void CanBus::stopReceiving()
{
    if (!udpSocket) return;

    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "stopReceiving", Qt::QueuedConnection);
        return;
    }

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
