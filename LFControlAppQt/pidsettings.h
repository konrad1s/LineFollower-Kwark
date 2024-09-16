#ifndef PIDSETTINGS_H
#define PIDSETTINGS_H

#include <cstring>
#include <cstdint>
#include <iostream>
#include "byteswap.h"

class PIDSettings
{
public:
    float kp, ki, kd;
    float integralMax, integralMin;
    float outputMax, outputMin;

    PIDSettings() = default;

    void parseFromArray(const uint8_t *data, bool isBigEndian = false)
    {
        std::size_t offset = 0;

        ByteSwap::copyAndSwapIfNeeded(kp, data + offset, isBigEndian);
        offset += sizeof(kp);
        ByteSwap::copyAndSwapIfNeeded(ki, data + offset, isBigEndian);
        offset += sizeof(ki);
        ByteSwap::copyAndSwapIfNeeded(kd, data + offset, isBigEndian);
        offset += sizeof(kd);
        ByteSwap::copyAndSwapIfNeeded(integralMax, data + offset, isBigEndian);
        offset += sizeof(integralMax);
        ByteSwap::copyAndSwapIfNeeded(integralMin, data + offset, isBigEndian);
        offset += sizeof(integralMin);
        ByteSwap::copyAndSwapIfNeeded(outputMax, data + offset, isBigEndian);
        offset += sizeof(outputMax);
        ByteSwap::copyAndSwapIfNeeded(outputMin, data + offset, isBigEndian);
        offset += sizeof(outputMin);
    }

    std::ostream& operator<<(std::ostream& os) const
    {
        os << "kp: " << kp << "\n";
        os << "ki: " << ki << "\n";
        os << "kd: " << kd << "\n";
        os << "integralMax: " << integralMax << "\n";
        os << "integralMin: " << integralMin << "\n";
        os << "outputMax: " << outputMax << "\n";
        os << "outputMin: " << outputMin << "\n";
        return os;
    }

    constexpr std::size_t size() const
    {
        return sizeof(kp) + sizeof(ki) + sizeof(kd) + sizeof(integralMax) +
               sizeof(integralMin) + sizeof(outputMax) + sizeof(outputMin);
    }
};

#endif //PIDSETTINGS_H
