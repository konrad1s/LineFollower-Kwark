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
    DebugData        = 0x0006,
    GetActiveSession = 0x0007,

    BootGetVersion      = 0xF001,
    BootStartDownload   = 0xF002,
    BootEraseApp        = 0xF003,
    BootFlashData       = 0xF004,
    BootFlashCrc        = 0xF005,
    BootValidateApp     = 0xF006,
    BootJumpToApp       = 0xF007,

    InvalidCommand  = 0xFFFF
};

enum class CommandSetMode : uint8_t
{
    Start = 0x00,
    Stop  = 0x01
};

#endif // COMMAND_H
