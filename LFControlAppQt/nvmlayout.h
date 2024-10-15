#ifndef NVMLAYOUT_H
#define NVMLAYOUT_H

#include "pidsettings.h"
#include <array>
#include <cstring>
#include <QString>

class NVMLayout
{
public:
    static constexpr size_t SENSORS_NUMBER = 12;

    PIDSettings pidStgSensor;
    PIDSettings pidStgEncoderLeft;
    PIDSettings pidStgEncoderRight;
    struct
    {
        std::array<int8_t, SENSORS_NUMBER> weights;
        std::array<uint16_t, SENSORS_NUMBER> thresholds;
        float errorThreshold;
        float fallbackErrorPositive;
        float fallbackErrorNegative;
    } sensors;
    float targetSpeed;

    NVMLayout() = default;

    void parseFromArray(const uint8_t *data)
    {
        size_t offset = 0;

        pidStgSensor.parseFromArray(data + offset);
        offset += pidStgSensor.size();

        pidStgEncoderLeft.parseFromArray(data + offset);
        offset += pidStgEncoderLeft.size();

        pidStgEncoderRight.parseFromArray(data + offset);
        offset += pidStgEncoderRight.size();

        std::memcpy(sensors.weights.data(), data + offset, sensors.weights.size() * sizeof(int8_t));
        offset += sensors.weights.size() * sizeof(int8_t);

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

        std::memcpy(&targetSpeed, data + offset, sizeof(targetSpeed));
        offset += sizeof(targetSpeed);
    }

    void serializeToArray(uint8_t *data) const
    {
        size_t offset = 0;

        pidStgSensor.serializeToArray(data + offset);
        offset += pidStgSensor.size();

        pidStgEncoderLeft.serializeToArray(data + offset);
        offset += pidStgEncoderLeft.size();

        pidStgEncoderRight.serializeToArray(data + offset);
        offset += pidStgEncoderRight.size();

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

        std::memcpy(data + offset, &targetSpeed, sizeof(targetSpeed));
        offset += sizeof(targetSpeed);
    }

    constexpr size_t size() const
    {
        return pidStgSensor.size() + pidStgEncoderLeft.size() + pidStgEncoderRight.size() +
               (sensors.weights.size() * sizeof(int8_t)) +
               (sensors.thresholds.size() * sizeof(uint16_t)) +
               sizeof(sensors.errorThreshold) + sizeof(sensors.fallbackErrorPositive) +
               sizeof(sensors.fallbackErrorNegative) + sizeof(targetSpeed);
    }

    QString toString() const
    {
        QString output;

        output.append("PID Sensor Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, "
                              "OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgSensor.kp)
                          .arg(pidStgSensor.ki)
                          .arg(pidStgSensor.kd)
                          .arg(pidStgSensor.integralMax)
                          .arg(pidStgSensor.integralMin)
                          .arg(pidStgSensor.outputMax)
                          .arg(pidStgSensor.outputMin));

        output.append("\nPID Encoder Left Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, "
                              "OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgEncoderLeft.kp)
                          .arg(pidStgEncoderLeft.ki)
                          .arg(pidStgEncoderLeft.kd)
                          .arg(pidStgEncoderLeft.integralMax)
                          .arg(pidStgEncoderLeft.integralMin)
                          .arg(pidStgEncoderLeft.outputMax)
                          .arg(pidStgEncoderLeft.outputMin));

        output.append("\nPID Encoder Right Settings:\n");
        output.append(QString("Kp: %1, Ki: %2, Kd: %3, IntMax: %4, IntMin: %5, "
                              "OutputMax: %6, OutputMin: %7\n")
                          .arg(pidStgEncoderRight.kp)
                          .arg(pidStgEncoderRight.ki)
                          .arg(pidStgEncoderRight.kd)
                          .arg(pidStgEncoderRight.integralMax)
                          .arg(pidStgEncoderRight.integralMin)
                          .arg(pidStgEncoderRight.outputMax)
                          .arg(pidStgEncoderRight.outputMin));

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
        output.append(QString("\nTarget Speed: %1\n").arg(targetSpeed));

        return output;
    }
};

#endif // NVMLAYOUT_H
