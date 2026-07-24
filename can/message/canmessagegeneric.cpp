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

ClickPoint CanMessageGeneric::GetCapturePointFromPayload()
{
    ClickPoint pt;

    pt.x = static_cast<uint16_t>(
        (static_cast<uint16_t>(Message.PL[1]) << 8) |
        static_cast<uint16_t>(Message.PL[0]));

    pt.y = static_cast<uint16_t>(
        (static_cast<uint16_t>(Message.PL[3]) << 8) |
        static_cast<uint16_t>(Message.PL[2]));

    return pt;
}


// void CanMessageGeneric::ParseUShort2()
// {
//     if (Node == static_cast<uint8_t>(NodeId::VIDEO_UNIT) /*&& Message.ACTION == 0*/) {
//         switch (Message.ID) {
//             case static_cast<uint8_t>(IdNode9::CAPTURE_POINT):
//                 ClickPoint pt = GetCapturePointFromPayload();
//                 break;
//         }
//     }
// }


// Передати координати точки для захоплення
// 0х198 00 00 0D 00 xx xx yy yy - xx xx - точка кліку по осі Х, yy yy - по осі Y
bool CanMessageGeneric::ParseCapturePoint(ClickPoint& pt)
{
    if (Node != static_cast<uint8_t>(NodeId::VIDEO_UNIT))
        return false;

    if (Message.ID != static_cast<uint8_t>(IdNode9::CAPTURE_POINT))
        return false;

    pt = GetCapturePointFromPayload();
    return true;
}

bool CanMessageGeneric::StopTrack()
{
    if (Node != static_cast<uint8_t>(NodeId::VIDEO_UNIT))
        return false;

    if (Message.ID != static_cast<uint8_t>(IdNode9::RESET_CAPTURE))
        return false;
    else return true;

}

bool CanMessageGeneric::ParseFOV(CameraFov &fov)
{
    fov = CameraFov{};
    if (Node != static_cast<uint8_t>(NodeId::VIDEO_UNIT))
        return false;

    if (static_cast<uint8_t>(Message.TYPE) != static_cast<uint8_t>(ParamType::Float))
        return false;

    if (Message.ACTION != 0x00)
        return false;

    if (Message.ID == static_cast<uint8_t>(IdNode9::FOV_H)) {
        fov.hDeg = GetFloatFromPayload();
        return true;
    }

    if (Message.ID == static_cast<uint8_t>(IdNode9::FOV_V)) {
        fov.vDeg = GetFloatFromPayload();
        return true;
    }

    return false;
}

bool CanMessageGeneric::ParseTrackingParams(TrackingParams &params)
{
    if (Node != static_cast<uint8_t>(NodeId::VIDEO_UNIT))
        return false;

    if (Message.ACTION != 0x00)
        return false;

    if (Message.ID == static_cast<uint8_t>(IdNode9::ROI_SIZE)) {
        if (Message.TYPE != ParamType::UShort)
            return false;

        const uint16_t value = GetUShortFromPayload();

        // Захист від дурних значень
        if (value < 10 || value > 1000)
            return false;
        params.roiSize = value;
        return true;
    }

    return false;
}
