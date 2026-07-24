#include "can/transport/SendDataFrame.h"
#include <QHostAddress>
#include <cstring>
#include <iostream>

SendDataFrame::SendDataFrame() {}

// Реалізація сінглтона
SendDataFrame &SendDataFrame::getInstance()
{
    static SendDataFrame instance;
    return instance;
}

std::vector<uint8_t> SendDataFrame::CreateHeaderCannelloni(uint8_t countPacks)
{
    std::vector<uint8_t> header;

    header.push_back(0x02);      // version
    header.push_back(0x00);      // type of frame
    header.push_back(seqNumber); // seq number
    header.push_back(0x00);      // number of CAN frames (2 bytes)
    header.push_back(countPacks);

    // Збільшуємо seqNumber з обгортанням
    seqNumber = (seqNumber == 0xFF) ? 0x01 : seqNumber + 1;

    return header;
}

void SendDataFrame::AddCanFrame(uint16_t can_id,
                                uint8_t can_len,
                                const std::vector<uint8_t> &payload)
{
    uint8_t can_id_high = (can_id >> 8) & 0xFF;
    uint8_t can_id_low = can_id & 0xFF;

    // Додаємо адресу CAN кадру: 0x00 0x00 0x02 0x38
    dataCanFrames.push_back(0x00);
    dataCanFrames.push_back(0x00);
    dataCanFrames.push_back(can_id_high);
    dataCanFrames.push_back(can_id_low);

    // Додаємо Count byte: 0x08 (8 байт в повідомленні)
    dataCanFrames.push_back(can_len);

    dataCanFrames.insert(dataCanFrames.end(), payload.begin(), payload.end());

    countPacks++;
}

void SendDataFrame::ClearCanFrame()
{
    dataCanFrames.clear();
    countPacks = 0;
}

int SendDataFrame::GetDataFrameLen()
{
    return countPacks;
}

void SendDataFrame::SendAllFrames()
{
    std::vector<uint8_t> header = CreateHeaderCannelloni(countPacks);
    dataCanFrames.insert(dataCanFrames.begin(), header.begin(), header.end());

    QByteArray byteArray(reinterpret_cast<const char *>(dataCanFrames.data()),
                         static_cast<int>(dataCanFrames.size()));

    udpSocket.writeDatagram(byteArray, QHostAddress("192.168.144.10"), 14500);

    std::cout << "Send by UDP: Count=" << countPacks << " [";
    for (size_t i = 0; i < dataCanFrames.size(); i++) {
        printf(" %02X", dataCanFrames[i]);
    }
    std::cout << " ]" << std::endl;



    QString hexString = byteArray.toHex(' ').toUpper();

    //qDebug() << "Send by UDP: Count=" << countPacks << "[" << hexString << "]";

    ClearCanFrame();
}

void SendDataFrame::Send(uint16_t can_id, uint8_t can_len, const std::vector<uint8_t> &payload)
{
    std::vector<uint8_t> data;
    // Додаємо Header

    std::vector<uint8_t> header = CreateHeaderCannelloni(1);
    data.insert(data.end(), header.begin(), header.end());

    uint8_t can_id_high = (can_id >> 8) & 0xFF;
    uint8_t can_id_low = can_id & 0xFF;

    // Додаємо Header: 0x02 0x00 0x6E 0x00 0x01
    // data.push_back(0x02);
    // data.push_back(0x00);
    // data.push_back(0x6E);
    // data.push_back(0x00);
    // data.push_back(0x01);

    // Додаємо адресу CAN кадру: 0x00 0x00 0x02 0x38
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(can_id_high);
    data.push_back(can_id_low);

    // Додаємо Count byte: 0x08 (8 байт в повідомленні)
    data.push_back(can_len);

    data.insert(data.end(), payload.begin(), payload.end());

    QByteArray byteArray(reinterpret_cast<const char *>(data.data()), static_cast<int>(data.size()));

    udpSocket.writeDatagram(byteArray, QHostAddress("192.168.144.10"), 14500);

    std::cout << "Send by UDP: ID=" << std::hex << can_id << " [";
    for (size_t i = 0; i < data.size(); i++) {
        printf(" %02X", data[i]);
    }
    std::cout << " ]" << std::endl;
}
