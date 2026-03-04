#include "canmessagegeneric.h"
//#include "laserparameters.h"
//#include "lpsparameters.h"

#include <cstring>

CanMessageGeneric::MESSAGE_t::MESSAGE_t(const std::vector<uint8_t> &message)
{
    PART_NUM = (message[5] & 0xF0) >> 4;
    PART_CNT = message[5] & 0x0F;
    ID = message[6];
    TYPE = static_cast<ParamType>(message[7]);
    ACTION = message[8];
    std::memcpy(PL, &message[9], 4);
}

CanMessageGeneric::CanMessageGeneric(const std::vector<uint8_t> &bytes)
    : BaseCanMessage(bytes)
    , Message(bytes)
{}

float CanMessageGeneric::GetFloatFromPayload()
{
    float floatValue;
    std::memcpy(&floatValue, Message.PL, sizeof(float));

    return floatValue;
}

// uint32_t CanMessageGeneric::GetULongFromPayload() {
//     uint32_t ulongValue;
//     std::memcpy(&ulongValue, Message.PL, sizeof(uint32_t));
//     return ulongValue;
// }

uint32_t CanMessageGeneric::GetULongFromPayload()
{
    uint32_t ulongValue = 0;
    ulongValue |= static_cast<uint32_t>(Message.PL[3]) << 0;
    ulongValue |= static_cast<uint32_t>(Message.PL[2]) << 8;
    ulongValue |= static_cast<uint32_t>(Message.PL[1]) << 16;
    ulongValue |= static_cast<uint32_t>(Message.PL[0]) << 24;
    return ulongValue;
}

int32_t CanMessageGeneric::GetLongFromPayload()
{
    int32_t longValue;
    std::memcpy(&longValue, Message.PL, sizeof(int32_t));
    return longValue;
}

uint16_t CanMessageGeneric::GetUShortFromPayload()
{
    uint16_t ushortValue;
    std::memcpy(&ushortValue, Message.PL, sizeof(uint16_t));
    return ushortValue;
}

int16_t CanMessageGeneric::GetShortFromPayload()
{
    int16_t shortValue;
    std::memcpy(&shortValue, Message.PL, sizeof(int16_t));
    return shortValue;
}

uint8_t CanMessageGeneric::GetByteFromPayload()
{
    uint8_t byteValue;
    std::memcpy(&byteValue, Message.PL, sizeof(uint8_t));
    return byteValue;
}

uint8_t CanMessageGeneric::ParseByte()
{

    uint8_t byteValue = GetByteFromPayload();
    /*LaserParameters &manager = LaserParameters::GetInstance();
    if (Node == static_cast<uint8_t>(NodeId::LASER_POINTER) && Message.ACTION == 0) {
        switch (Message.ID) {
        case static_cast<uint8_t>(IdNode4::LASER_ACTIVE):
            manager.SetLaserActive(byteValue);
            break;
        case static_cast<uint8_t>(IdNode4::PULSE_ON):
            manager.SetPulseOn(byteValue);
            break;
        case static_cast<uint8_t>(IdNode4::THERMAL_CONTROLE):
            manager.SetThermocontrolOn(byteValue);
            break;
        case static_cast<uint8_t>(IdNode4::BLIND_ON):
            manager.SetBlindOn(byteValue);
            break;
        default:
            break;
        }
    }
*/
    return byteValue;
}

