#ifndef NVMLAYOUT_H
#define NVMLAYOUT_H

#include "pidsettings.h"
#include "byteswap.h"
#include <array>

class NVMLayout
{
public:
    static constexpr size_t SENSORS_NUMBER = 12;

    PIDSettings pidStgSensor;
    PIDSettings pidStgMotorLeft;
    PIDSettings pidStgMotorRight;
    std::array<int8_t, SENSORS_NUMBER> sensorWeights;
    float errorThreshold;
    float fallbackErrorPositive;
    float fallbackErrorNegative;

    NVMLayout() = default;

    void parseFromArray(const uint8_t *data, bool isBigEndian = false)
    {
        size_t offset = 0;

        pidStgSensor.parseFromArray(data + offset, isBigEndian);
        offset += pidStgSensor.size();

        pidStgMotorLeft.parseFromArray(data + offset, isBigEndian);
        offset += pidStgMotorLeft.size();

        pidStgMotorRight.parseFromArray(data + offset, isBigEndian);
        offset += pidStgMotorRight.size();

        std::memcpy(sensorWeights.data(), data + offset, sensorWeights.size() * sizeof(int8_t));
        offset += sensorWeights.size() * sizeof(int8_t);

        ByteSwap::copyAndSwapIfNeeded(errorThreshold, data + offset, isBigEndian);
        offset += sizeof(float);

        ByteSwap::copyAndSwapIfNeeded(fallbackErrorPositive, data + offset, isBigEndian);
        offset += sizeof(float);

        ByteSwap::copyAndSwapIfNeeded(fallbackErrorNegative, data + offset, isBigEndian);
    }

    constexpr size_t size() const
    {
        return pidStgSensor.size() + pidStgMotorLeft.size() + pidStgMotorRight.size() + sensorWeights.size() +
               sizeof(errorThreshold) + sizeof(fallbackErrorPositive) + sizeof(fallbackErrorNegative);
    }
};

#endif //NVMLAYOUT_H
