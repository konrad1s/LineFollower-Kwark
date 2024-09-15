#include <cstring>
#include <cstdint>
#include <iostream>

class PIDSettings
{
public:
    float kp, ki, kd;
    float integral_max, integral_min;
    float output_max, output_min;

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

        std::memcpy(&integral_max, data + offset, sizeof(integral_max));
        offset += sizeof(integral_max);

        std::memcpy(&integral_min, data + offset, sizeof(integral_min));
        offset += sizeof(integral_min);

        std::memcpy(&output_max, data + offset, sizeof(output_max));
        offset += sizeof(output_max);

        std::memcpy(&output_min, data + offset, sizeof(output_min));
    }

    std::ostream& operator<<(std::ostream& os) const
    {
        os << "kp: " << kp << "\n";
        os << "ki: " << ki << "\n";
        os << "kd: " << kd << "\n";
        os << "integral_max: " << integral_max << "\n";
        os << "integral_min: " << integral_min << "\n";
        os << "output_max: " << output_max << "\n";
        os << "output_min: " << output_min << "\n";
        return os;
    }
};