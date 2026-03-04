#ifndef BASECANMESSAGE_H
#define BASECANMESSAGE_H

#include <cstdint>
#include <cstring>
#include <vector>

class BaseCanMessage
{
public:
    uint16_t Address;
    uint8_t Node;
    uint8_t Dir;
    uint8_t ParamID;
    uint8_t Priority;
    //uint8_t Payload[8];
    std::vector<uint8_t> *Payload;

    BaseCanMessage(const std::vector<uint8_t> &bytes);

protected:
    void ParseAddress();
};

#endif // BASECANMESSAGE_H
