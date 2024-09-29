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

    DebugData() = default;

    void parseFromArray(const uint8_t *data)
    {
        size_t offset = 0;

        for (size_t i = 0U; i < sensorValues.size(); ++i)
        {
            sensorValues[i] = static_cast<uint16_t>(data[offset]) |
                              (static_cast<uint16_t>(data[offset + 1]) << 8);
            offset += sizeof(sensorValues[i]);
        }
        sensorError = *reinterpret_cast<const float *>(data + offset);
        offset += sizeof(sensorError);
    }

    constexpr size_t size() const
    {
        return sizeof(sensorError) + (sensorValues.size() * sizeof(uint16_t));
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

        return output;
    }
};

#endif // DEBUGDATA_H
