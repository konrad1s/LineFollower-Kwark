#ifndef DEBUGDATA_H
#define DEBUGDATA_H

#include <stdint.h>
#include <cstddef>
#include <array>

class DebugData
{
public:
    static constexpr size_t SENSORS_NUMBER = 12;

    std::array<uint16_t, SENSORS_NUMBER> sensorValues;
    float sensorError;
    float motorLeftVelocity;
    float motorRightVelocity;
    bool isSpeedReduced;

    DebugData() = default;

    void parseFromArray(const uint8_t *data)
    {
        size_t offset = 0;

        for (size_t i = 0U; i < sensorValues.size(); ++i)
        {
            std::memcpy(&sensorValues[i], data + offset, sizeof(sensorValues[i]));
            offset += sizeof(sensorValues[i]);
        }
        sensorError = *reinterpret_cast<const float *>(data + offset);
        offset += sizeof(sensorError);
        motorLeftVelocity = *reinterpret_cast<const float *>(data + offset);
        offset += sizeof(sensorError);
        motorRightVelocity = *reinterpret_cast<const float *>(data + offset);
        offset += sizeof(sensorError);
        isSpeedReduced = *reinterpret_cast<const bool *>(data + offset);
        offset += sizeof(isSpeedReduced);
    }

    constexpr size_t size() const
    {
        return sizeof(sensorError) + (sensorValues.size() * sizeof(uint16_t)) +
               sizeof(motorLeftVelocity) + sizeof(motorRightVelocity) + sizeof(isSpeedReduced);
    }

    QString toString() const
    {
        QString output;
        output.append("Debug Data Sensor Values:\n");

        for (size_t i = 0U; i < sensorValues.size(); ++i)
        {
            output.append(QString("Sensor %1 Value: %2\n")
                              .arg(i + 1)
                              .arg(sensorValues[i]));
        }
        output.append(QString("Sensor Error: %1\n").arg(sensorError));
        output.append(QString("Motor Left Velocity: %1\n").arg(motorLeftVelocity));
        output.append(QString("Motor Right Velocity: %1\n").arg(motorRightVelocity));
        output.append(QString("Right angle detected: %1\n").arg(isSpeedReduced));

        return output;
    }
};

#endif // DEBUGDATA_H
