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
    struct
    {
        std::array<int8_t, SENSORS_NUMBER> weights;
        std::array<uint16_t, SENSORS_NUMBER> thresholds;
        float errorThreshold;
        float fallbackErrorPositive;
        float fallbackErrorNegative;
    } sensors;

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

        std::memcpy(sensors.weights.data(), data + offset, sensors.weights.size() * sizeof(int8_t));
        offset +=  sensors.weights.size() * sizeof(int8_t);

        for (size_t i = 0U; i < sensors.thresholds.size(); ++i)
        {
            ByteSwap::copyAndSwapIfNeeded(sensors.thresholds[i], data + offset, isBigEndian);
            offset += sizeof(sensors.thresholds[i]);
        }

        ByteSwap::copyAndSwapIfNeeded(sensors.errorThreshold, data + offset, isBigEndian);
        offset += sizeof(float);

        ByteSwap::copyAndSwapIfNeeded(sensors.fallbackErrorPositive, data + offset, isBigEndian);
        offset += sizeof(float);

        ByteSwap::copyAndSwapIfNeeded(sensors.fallbackErrorNegative, data + offset, isBigEndian);
    }

    constexpr size_t size() const
    {
        return pidStgSensor.size() + pidStgMotorLeft.size() + pidStgMotorRight.size() +
               (sensors.weights.size() * sizeof(int8_t)) +
               (sensors.thresholds.size() * sizeof(uint16_t)) +
               sizeof(sensors.errorThreshold) + sizeof(sensors.fallbackErrorPositive) + sizeof(sensors.fallbackErrorNegative);
    }
};

#endif //NVMLAYOUT_H
