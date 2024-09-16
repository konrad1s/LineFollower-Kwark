#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>

enum class Command : uint16_t
{
    SetMode          = 0x0000,
    Reset            = 0x0002,
    Calibrate        = 0x0004,
    ReadNvmData      = 0x0006,
    WriteNvmData     = 0x0008,
    SetDebugMode     = 0x0010,
    SetPID           = 0x0100,
    SetSensorWeights = 0x0102,
    GetSensorWeights = 0x0200,

    InvalidCommand   = 0xFFFF
};

#endif // COMMAND_H
