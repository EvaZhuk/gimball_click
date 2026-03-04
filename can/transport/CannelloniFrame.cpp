#include "CannelloniFrame.h"

#include <iostream>
#include <stdexcept>

CannelloniFrame::CannelloniFrame(const QByteArray &data)
{
    std::vector<uint8_t> bytes(data.begin(),
                               data.end()); // Конвертація QByteArray у std::vector<uint8_t>
    if (bytes.size() < 5) {
        throw std::invalid_argument("Invalid CannelloniFrame: Too short");
    }

    Version = bytes[0];
    OPCode = bytes[1];
    SeqNo = bytes[2];
    Count = ((bytes[3] << 8) | bytes[4]);

    ParseCANMessages(bytes);
}

void CannelloniFrame::ParseCANMessages(const std::vector<uint8_t> &bytes)
{
    size_t offset = 5;

    for (int i = 0; i < Count && offset + 6 <= bytes.size(); i++) {
        CANMessage msg;
        size_t messageSize = 5;
        if (offset + messageSize > bytes.size()) {
            throw std::runtime_error("Invalid CAN message header");
        }

        msg.can_id = (bytes[offset + 2] << 8) | (bytes[offset + 3]);
        msg.len = bytes[offset + 4];
        messageSize += msg.len;

        if (offset + messageSize > bytes.size()) {
            throw std::runtime_error("Invalid CAN message length");
        }

        msg.raw_data.insert(msg.raw_data.end(),
                            bytes.begin() + offset,
                            bytes.begin() + offset + messageSize);
        offset += messageSize;

        messageQueue.push(msg.raw_data);
        messages.push_back(msg);
    }
}

void CannelloniFrame::PrintCANMessages() const
{
    std::cout << "Extracted CAN Messages:\n";
    for (const auto &msg : messages) {
        for (size_t i = 0; i < msg.raw_data.size(); ++i) {
            std::cout << std::hex << static_cast<int>(msg.raw_data[i]) << " ";
        }
        std::cout << std::endl;
    }
}
