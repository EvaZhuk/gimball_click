#include "mavlinkinterface.h"
#include <QDebug>

MavlinkInterface::MavlinkInterface(QObject *parent)
    : QObject(parent)
{
    connect(&serial, &QSerialPort::readyRead, [&]() {
        QByteArray data = serial.readAll();
        for (uint8_t b : data)
            processByte(b);
    });
}

void MavlinkInterface::start()
{
    serial.setPortName("/dev/ttyS4");
    serial.setBaudRate(QSerialPort::Baud115200);

    if (!serial.open(QIODevice::ReadWrite)) {
        qDebug() << "Не можу відкрити порт";
        return;
    }

    qDebug() << "Порт відкрито. Очікуємо HEARTBEAT...";
}

void MavlinkInterface::processByte(uint8_t byte)
{
    static mavlink_message_t msg;
    static mavlink_status_t status;

    if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status)) {

        switch (msg.msgid) {

        case MAVLINK_MSG_ID_HEARTBEAT: {
            mavlink_heartbeat_t hb;
            mavlink_msg_heartbeat_decode(&msg, &hb);

            qDebug() << "HEARTBEAT отримано. Режим:" << hb.custom_mode;

            if (!heartbeat_received) {
                heartbeat_received = true;
                sendRequestDataStream();
                sendStatustext();
                setFlightMode();
            }
        } break;


        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
            mavlink_global_position_int_t pos;
            mavlink_msg_global_position_int_decode(&msg, &pos);

            qDebug() << "GPS:"
                     << pos.lat / 1e7
                     << pos.lon / 1e7
                     << "Alt:" << pos.relative_alt / 1e3;
        } break;


        case MAVLINK_MSG_ID_ATTITUDE: {
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(&msg, &att);

            qDebug() << "ATT:"
                     << att.roll  * 57.2958
                     << att.pitch * 57.2958
                     << att.yaw   * 57.2958;
        } break;


        case MAVLINK_MSG_ID_STATUSTEXT: {
            mavlink_statustext_t st;
            mavlink_msg_statustext_decode(&msg, &st);

            qDebug() << "STATUSTEXT:" << st.text;
        } break;

        }
    }
}

void MavlinkInterface::sendRequestDataStream()
{
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    mavlink_msg_request_data_stream_pack(
        sys_id, comp_id, &msg,
        1, 1,                      // Pixhawk
        MAV_DATA_STREAM_ALL,
        10,                        // 10 Hz
        1
        );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    serial.write((char*)buf, len);

    qDebug() << "Запит телеметрії надіслано";
}

void MavlinkInterface::sendStatustext()
{
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    const char *text = "Test message from Qt";

    mavlink_msg_statustext_pack(
        sys_id, comp_id, &msg,
        MAV_SEVERITY_INFO,
        text,
        0, 0
        );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    serial.write((char*)buf, len);

    qDebug() << "STATUSTEXT sent";
}

void MavlinkInterface::setFlightMode()
{
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    int guided = 4; // GUIDED

    mavlink_msg_set_mode_pack(
        sys_id, comp_id, &msg,
        1,                               // Pixhawk
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        guided
        );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    serial.write((char*)buf, len);

    qDebug() << "GUIDED mode set";
}
