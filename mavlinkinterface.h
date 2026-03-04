#ifndef MAVLINKINTERFACE_H
#define MAVLINKINTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>

extern "C" {
#include "/home/lps/c_library_v2/common/mavlink.h"
}

class MavlinkInterface : public QObject
{
    Q_OBJECT
public:
    explicit MavlinkInterface(QObject *parent = nullptr);

    void start();     // запускає з'єднання
    void sendRequestDataStream();
    void sendStatustext();
    void setFlightMode();

private:
    QSerialPort serial;
    uint8_t sys_id = 255;
    uint8_t comp_id = MAV_COMP_ID_ONBOARD_COMPUTER;
    bool heartbeat_received = false;

    void processByte(uint8_t byte);
};

#endif // MAVLINKINTERFACE_H
