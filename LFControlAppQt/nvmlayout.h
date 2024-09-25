#ifndef NVMLAYOUT_H
#define NVMLAYOUT_H

#include "pidsettings.h"
#include <array>
#include <cstring>

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

    void parseFromArray(const uint8_t *data)
    {
        size_t offset = 0;

        pidStgSensor.parseFromArray(data + offset);
        offset += pidStgSensor.size();

        pidStgMotorLeft.parseFromArray(data + offset);
        offset += pidStgMotorLeft.size();

        pidStgMotorRight.parseFromArray(data + offset);
        offset += pidStgMotorRight.size();

        std::memcpy(sensors.weights.data(), data + offset, sensors.weights.size() * sizeof(int8_t));
        offset +=  sensors.weights.size() * sizeof(int8_t);

        for (size_t i = 0U; i < sensors.thresholds.size(); ++i)
        {
            std::memcpy(&sensors.thresholds[i], data + offset, sizeof(sensors.thresholds[i]));
            offset += sizeof(sensors.thresholds[i]);
        }

        std::memcpy(&sensors.errorThreshold, data + offset, sizeof(sensors.errorThreshold));
        offset += sizeof(sensors.errorThreshold);

        std::memcpy(&sensors.fallbackErrorPositive, data + offset, sizeof(sensors.fallbackErrorPositive));
        offset += sizeof(sensors.fallbackErrorPositive);

        std::memcpy(&sensors.fallbackErrorNegative, data + offset, sizeof(sensors.fallbackErrorNegative));
        offset += sizeof(sensors.fallbackErrorNegative);
    }

    void serializeToArray(uint8_t *data) const
    {
        size_t offset = 0;

        pidStgSensor.serializeToArray(data + offset);
        offset += pidStgSensor.size();

        pidStgMotorLeft.serializeToArray(data + offset);
        offset += pidStgMotorLeft.size();

        pidStgMotorRight.serializeToArray(data + offset);
        offset += pidStgMotorRight.size();

        std::memcpy(data + offset, sensors.weights.data(), sensors.weights.size() * sizeof(int8_t));
        offset += sensors.weights.size() * sizeof(int8_t);

        for (size_t i = 0U; i < sensors.thresholds.size(); ++i)
        {
            std::memcpy(data + offset, &sensors.thresholds[i], sizeof(sensors.thresholds[i]));
            offset += sizeof(sensors.thresholds[i]);
        }

        std::memcpy(data + offset, &sensors.errorThreshold, sizeof(sensors.errorThreshold));
        offset += sizeof(sensors.errorThreshold);

        std::memcpy(data + offset, &sensors.fallbackErrorPositive, sizeof(sensors.fallbackErrorPositive));
        offset += sizeof(sensors.fallbackErrorPositive);

        std::memcpy(data + offset, &sensors.fallbackErrorNegative, sizeof(sensors.fallbackErrorNegative));
        offset += sizeof(sensors.fallbackErrorNegative);
    }

    constexpr size_t size() const
    {
        return pidStgSensor.size() + pidStgMotorLeft.size() + pidStgMotorRight.size() +
               (sensors.weights.size() * sizeof(int8_t)) +
               (sensors.thresholds.size() * sizeof(uint16_t)) +
               sizeof(sensors.errorThreshold) + sizeof(sensors.fallbackErrorPositive) + sizeof(sensors.fallbackErrorNegative);
    }

    QString toString() const
    {
        QString output;

        output.append("PID Sensor Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgSensor.kp)
                          .arg(pidStgSensor.ki)
                          .arg(pidStgSensor.kd)
                          .arg(pidStgSensor.integralMax)
                          .arg(pidStgSensor.integralMin)
                          .arg(pidStgSensor.outputMax)
                          .arg(pidStgSensor.outputMin));

        output.append("\nPID Motor Left Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgMotorLeft.kp)
                          .arg(pidStgMotorLeft.ki)
                          .arg(pidStgMotorLeft.kd)
                          .arg(pidStgMotorLeft.integralMax)
                          .arg(pidStgMotorLeft.integralMin)
                          .arg(pidStgMotorLeft.outputMax)
                          .arg(pidStgMotorLeft.outputMin));

        output.append("\nPID Motor Right Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgMotorRight.kp)
                          .arg(pidStgMotorRight.ki)
                          .arg(pidStgMotorRight.kd)
                          .arg(pidStgMotorRight.integralMax)
                          .arg(pidStgMotorRight.integralMin)
                          .arg(pidStgMotorRight.outputMax)
                          .arg(pidStgMotorRight.outputMin));

        output.append("\nSensor Weights:\n");
        for (int i = 0; i < SENSORS_NUMBER; ++i)
        {
            output.append(QString("Sensor %1 Weight: %2, Threshold: %3\n")
                              .arg(i + 1)
                              .arg(sensors.weights[i])
                              .arg(sensors.thresholds[i]));
        }

        output.append(QString("\nError Threshold: %1\n").arg(sensors.errorThreshold));
        output.append(QString("Fallback Error Positive: %1\n").arg(sensors.fallbackErrorPositive));
        output.append(QString("Fallback Error Negative: %1\n").arg(sensors.fallbackErrorNegative));

        return output;
    }
};

#endif //NVMLAYOUT_H
