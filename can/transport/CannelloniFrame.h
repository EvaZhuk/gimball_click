#pragma once

#include <QByteArray>
#include <cstdint>
#include <cstring>
#include <queue>
#include <vector>

class CannelloniFrame
{
public:
    uint8_t Version;
    uint8_t OPCode;
    uint8_t SeqNo;
    uint16_t Count;

    struct CANMessage
    {
        uint32_t can_id;
        uint8_t len;
        std::vector<uint8_t> raw_data;
    };

    std::vector<CANMessage> messages;

    CannelloniFrame(const QByteArray &data); // Тепер приймає QByteArray
    void PrintCANMessages() const;
    void PrintMessageQueue() const;

    std::queue<std::vector<uint8_t>> GetMessageQueue() const { return messageQueue; }

protected:
    void ParseCANMessages(const std::vector<uint8_t> &bytes);
    std::queue<std::vector<uint8_t>> messageQueue;
};
