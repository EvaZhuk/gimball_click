#ifndef SENDDATAFRAME_H
#define SENDDATAFRAME_H

#include <QObject>
#include <QUdpSocket>
#include <vector>

class SendDataFrame
{
public:
    static SendDataFrame &getInstance(); // Метод доступу до єдиного екземпляра
    void Send(
        uint16_t can_id,
        uint8_t can_len,
        const std::vector<uint8_t> &
            payload); // Метод відправки CAN повідомлення'void SendDataFrame::CreateHeaderCannelloni(){
    //void CreateHeaderCannelloni(uint8_t lastByte);
    void AddCanFrame(uint16_t can_id, uint8_t can_len, const std::vector<uint8_t> &payload);
    void SendAllFrames();
    int GetDataFrameLen();

private:
    SendDataFrame();                               // Приватний конструктор
    SendDataFrame(const SendDataFrame &) = delete; // Заборонити копіювання
    SendDataFrame &operator=(const SendDataFrame &) = delete;

    QUdpSocket udpSocket;
    std::vector<uint8_t> dataCanFrames;
    std::vector<uint8_t> CreateHeaderCannelloni(uint8_t countPacks);
    void ClearCanFrame();

    uint8_t seqNumber = 0x01; // Лічильник seq
    uint16_t countPacks = 0;  //Лічильник CAN Packets
};

#endif // SENDDATAFRAME_H
