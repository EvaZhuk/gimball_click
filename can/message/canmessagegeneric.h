#ifndef CANMESSAGEGENERIC_H
#define CANMESSAGEGENERIC_H
#include "can/message/basecanmessage.h"
#include <cstdint>
#include <vector>
//#include "AngleManager.h"
//#include "SpeedManager.h"
//#include "RangeManager.h"
enum class ParamType {
    NoneType,
    ErrorType,
    Float,
    Long,
    ULong,
    BLong,
    Short,
    UShort,
    BShort,
    Char,
    UChar,
    BChar,
    Short2,
    UShort2,
    BShort2,
    Char4,
    UChar4,
    BChar4,
    Char2,
    UChar2,
    BChar2,
    MEMID,
    CHIKSUM,
    AChar,
    AChar2,
    AChar4,
    Char3,
    UChar3,
    BChar3,
    AChar3,
    DoubleH,
    DoubleL,
};

enum class NodeId { PLATFORM = 1, GYROVERTICAL, RANGE_FINDER, LASER_POINTER, THERMAL_CAMERA = 8, VIDEO_UNIT = 9 };

enum class IdNode1 {
    MODE = 0,
    SET_HORIZONTAL_CHANNEL = 0x10,
    ANGLE_HORIZONTAL_CHANNEL = 0x11,
    ZERO_SET_HORIZONTAL_ENCODER = 0x12,
    ZERO_RESET_HORIZONTAL_ENCODER = 0x13,
    ENABLE_HORIZONTAL_ENCODER = 0x14,
    SET_PROGRAM_ZERO_HORIZONTAL_ENCODER = 0x15,

    SET_VERTICAL_CHANNEL = 0x20,
    ANGLE_VERTICAL_CHANNEL = 0x21,
    ZERO_SET_VERTICAL_ENCODER = 0x22,
    ZERO_RESET_VERTICAL_ENCODER = 0x23,
    ENABLE_VERTICAL_ENCODER = 0x24,
    SET_PROGRAM_ZERO_VERTICAL_ENCODER = 0x25
};

enum class IdNode3 { MEASURED_RANGES_1 = 0x06 };

enum class IdNode4 {
    LASER_ACTIVE = 0,
    PULSE_ON = 0x1,
    FREQUENCY = 0x2,
    STANAG = 0x3,
    TIME_RADIATION_REMAINING = 0x4,
    TIME_DELAY_REMAINING = 0x5,
    PULSE_CYCLE_COUNT = 0x6,
    ENERGY = 0x0E,
    BLIND_ON = 0x0F,
    THERMAL_CONTROLE = 0xC0,
    TEMPERATURE = 0xC1,
    COUNT_PULSES = 0xF0,
    ERROR_CODE = 0xFE
};

enum class IdNode9 { CAPTURE_POINT = 0x0, RESET_CAPTURE = 0x1 };

struct ClickPoint
{
    uint16_t x = 0;
    uint16_t y = 0;
};

class CanMessageGeneric : public BaseCanMessage
{
public:
    struct MESSAGE_t
    {
        uint8_t PART_NUM;
        uint8_t PART_CNT;
        uint8_t ID;
        ParamType TYPE;
        uint8_t ACTION;
        uint8_t PL[4];

        MESSAGE_t(const std::vector<uint8_t> &message);
    };

    MESSAGE_t Message;

    CanMessageGeneric(const std::vector<uint8_t> &bytes);

    float ParseFloat();
    uint32_t ParseULong();
    uint8_t ParseByte();

    void PrintParsedValue();
    float GetFloatFromPayload();
    uint32_t GetULongFromPayload();
    int32_t GetLongFromPayload();
    uint16_t GetUShortFromPayload();
    int16_t GetShortFromPayload();
    uint8_t GetByteFromPayload();
    void ParseUShort2();
    bool ParseCapturePoint(ClickPoint &pt);
    bool StopTrack();
private:
    ClickPoint GetCapturePointFromPayload();

};

#endif // CANMESSAGEGENERIC_H
