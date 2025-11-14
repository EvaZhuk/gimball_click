#include "siyisender.h"
#include <QDebug>
#include <algorithm> // Для std::clamp

SiyiSender::SiyiSender(QObject *parent) : QObject(parent) {
    //udp.bind(QHostAddress::AnyIPv4, 0); // Прив'язка до будь-якого доступного порту
    udp.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::DefaultForPlatform);

}

void SiyiSender::setTarget(const QHostAddress &ip, quint16 port) {
    targetIP = ip;
    targetPort = port;
}

// Функція для відправки команд швидкості до гімбала Siyi A8 Mini
// Використовує CMD_ID 0x07 (Gimbal Rotation Control) з int8_t значеннями -100 до 100.
void SiyiSender::sendSpeeds(float yaw_speed_deg_sec, float pitch_speed_deg_sec, float roll_speed_deg_sec) {
    // Масштабування вхідних швидкостей (deg/sec) до діапазону -100..100 для протоколу.
    // Приклад: якщо MAX_GIMBAL_SPEED_DEG_SEC = 30, то 30 deg/sec = 100, -30 deg/sec = -100.
    int8_t yaw_scaled = static_cast<int8_t>(std::clamp(
        yaw_speed_deg_sec / MAX_GIMBAL_SPEED_DEG_SEC * MAX_PROTOCOL_VALUE,
        static_cast<float>(-MAX_PROTOCOL_VALUE), static_cast<float>(MAX_PROTOCOL_VALUE)
        ));

    int8_t pitch_scaled = static_cast<int8_t>(std::clamp(
        pitch_speed_deg_sec / MAX_GIMBAL_SPEED_DEG_SEC * MAX_PROTOCOL_VALUE,
        static_cast<float>(-MAX_PROTOCOL_VALUE), static_cast<float>(MAX_PROTOCOL_VALUE)
        ));

    int8_t roll_scaled = static_cast<int8_t>(std::clamp(
        roll_speed_deg_sec / MAX_GIMBAL_SPEED_DEG_SEC * MAX_PROTOCOL_VALUE,
        static_cast<float>(-MAX_PROTOCOL_VALUE), static_cast<float>(MAX_PROTOCOL_VALUE)
        ));

    QByteArray packet;
    packet.append(char(0x55)); // Заголовок 1
    packet.append(char(0x66)); // Заголовок 2
    packet.append(char(0x01)); // Загальний номер версії (General version number)
    packet.append(char(0x03)); // Довжина даних (3 байти: yaw, pitch, roll - int8_t кожен)
    packet.append(char(0x00)); // SEQ (послідовність)
    packet.append(char(0x00)); // SEQ
    packet.append(char(0x00)); // CMD_ID (старший байт)
    packet.append(char(0x07)); // CMD_ID (молодший байт) - Gimbal Rotation Control

    // Yaw Speed (int8_t)
    packet.append(char(yaw_scaled));

    // Pitch Speed (int8_t)
    packet.append(char(pitch_scaled));

    // Roll Speed (int8_t)
    packet.append(char(roll_scaled));

    // Розрахунок CRC16
    uint16_t crc = calculateCRC16(reinterpret_cast<const uint8_t*>(packet.data()), packet.size());
    packet.append(char(crc & 0xFF));        // CRC молодший байт
    packet.append(char((crc >> 8) & 0xFF)); // CRC старший байт

    qint64 bytesSent = udp.writeDatagram(packet, targetIP, targetPort);
    if (bytesSent == -1) {
        qDebug() << "Error sending UDP packet:" << udp.errorString();
    } else {
        qDebug() << "[SIYI] Sent speeds -> Yaw:" << yaw_speed_deg_sec << "(" << yaw_scaled << ")"
                 << "Pitch:" << pitch_speed_deg_sec << "(" << pitch_scaled << ")"
                 << "Roll:" << roll_speed_deg_sec << "(" << roll_scaled << ")";
    }
}

// Функція розрахунку CRC16 (поліном 0x1021, початкове значення 0, без рефлексії)
uint16_t SiyiSender::calculateCRC16(const uint8_t* data, int length) {
    uint16_t crc = 0;
    for (int i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}
