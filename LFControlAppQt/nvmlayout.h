#include "pidsetting.h"
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

    void parseFromArray(const uint8_t *data)
    {
        size_t offset = 0;

        pidStgSensor.parseFromArray(data + offset);
        offset += sizeof(PIDSettings);

        pidStgMotorLeft.parseFromArray(data + offset);
        offset += sizeof(PIDSettings);

        pidStgMotorRight.parseFromArray(data + offset);
        offset += sizeof(PIDSettings);

        std::memcpy(sensorWeights.data(), data + offset, sensorWeights.size() * sizeof(int8_t));
        offset += sensorWeights.size() * sizeof(int8_t);

        std::memcpy(&errorThreshold, data + offset, sizeof(float));
        offset += sizeof(float);

        std::memcpy(&fallbackErrorPositive, data + offset, sizeof(float));
        offset += sizeof(float);

        std::memcpy(&fallbackErrorNegative, data + offset, sizeof(float));
    }

};
