#include "basecanmessage.h"
#include <vector>

BaseCanMessage::BaseCanMessage(const std::vector<uint8_t> &bytes)
{
    //Address = static_cast<uint16_t>(bytes[2] | (bytes[3] << 8));
    Address = (bytes[2] << 8) | (bytes[3]);
    ParseAddress();
    //Payload.resize(8);
    Payload = new std::vector<uint8_t>(8);
    std::memcpy(Payload->data(), &bytes[5], 8);
}

void BaseCanMessage::ParseAddress()
{
    Node = (Address >> 4) & 0x0F;     // Extract bits 4-7
    Dir = (Address >> 3) & 0x01;      // Extract bit 3
    Priority = (Address >> 8) & 0x07; // Extract bits 8-10
    ParamID = Address & 0x07;         // Extract bits 0-2
}
