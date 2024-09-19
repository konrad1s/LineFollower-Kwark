#ifndef PIDSETTINGS_H
#define PIDSETTINGS_H

#include <cstring>
#include <cstdint>
#include <iostream>

class PIDSettings
{
public:
    float kp, ki, kd;
    float integralMax, integralMin;
    float outputMax, outputMin;

    PIDSettings() = default;

    void parseFromArray(const uint8_t *data)
    {
        std::size_t offset = 0;

        std::memcpy(&kp, data + offset, sizeof(kp));
        offset += sizeof(kp);

        std::memcpy(&ki, data + offset, sizeof(ki));
        offset += sizeof(ki);

        std::memcpy(&kd, data + offset, sizeof(kd));
        offset += sizeof(kd);

        std::memcpy(&integralMax, data + offset, sizeof(integralMax));
        offset += sizeof(integralMax);

        std::memcpy(&integralMin, data + offset, sizeof(integralMin));
        offset += sizeof(integralMin);

        std::memcpy(&outputMax, data + offset, sizeof(outputMax));
        offset += sizeof(outputMax);

        std::memcpy(&outputMin, data + offset, sizeof(outputMin));
        offset += sizeof(outputMin);
    }

    void serializeToArray(uint8_t *data) const
    {
        std::size_t offset = 0;

        std::memcpy(data + offset, &kp, sizeof(kp));
        offset += sizeof(kp);

        std::memcpy(data + offset, &ki, sizeof(ki));
        offset += sizeof(ki);

        std::memcpy(data + offset, &kd, sizeof(kd));
        offset += sizeof(kd);

        std::memcpy(data + offset, &integralMax, sizeof(integralMax));
        offset += sizeof(integralMax);

        std::memcpy(data + offset, &integralMin, sizeof(integralMin));
        offset += sizeof(integralMin);

        std::memcpy(data + offset, &outputMax, sizeof(outputMax));
        offset += sizeof(outputMax);

        std::memcpy(data + offset, &outputMin, sizeof(outputMin));
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
