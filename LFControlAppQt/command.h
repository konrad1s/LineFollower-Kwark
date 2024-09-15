#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>

enum class Command : uint16_t
{
    SetMode          = 0x0000,
    Reset            = 0x0001,
    Calibrate        = 0x0002,
    ReadNvmData      = 0x0003,
    WriteNvmData     = 0x0004,
    SetDebugMode     = 0x0005,
    SetPID           = 0x0100,
    SetSensorWeights = 0x0101,
    GetSensorWeights = 0x0201,

    InvalidCommand   = 0xFFFF
};

#endif // COMMAND_H
