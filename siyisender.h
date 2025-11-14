#ifndef SIYISENDER_H
#define SIYISENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class SiyiSender : public QObject
{
    Q_OBJECT

public:
    explicit SiyiSender(QObject *parent = nullptr);

    void setTarget(const QHostAddress &ip, quint16 port);
    // Відправляє команди швидкості (градуси/сек) для Yaw, Pitch, Roll
    // Внутрішньо конвертує ці значення в діапазон -100 до 100 для протоколу Siyi
    void sendSpeeds(float yaw_speed_deg_sec, float pitch_speed_deg_sec, float roll_speed_deg_sec = 0.0f);

private:
    QUdpSocket udp;
    QHostAddress targetIP = QHostAddress("192.168.144.25");
    quint16 targetPort = 37260;

    // Максимальна швидкість, яку ми відправляємо в deg/sec
    // Використовується для масштабування в діапазон -100..100
    const float MAX_GIMBAL_SPEED_DEG_SEC = 30.0f;
    const int MAX_PROTOCOL_VALUE = 100; // Діапазон значень для протоколу Siyi

    uint16_t calculateCRC16(const uint8_t* data, int length);
};

#endif // SIYISENDER_H
