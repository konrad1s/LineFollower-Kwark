#ifndef BYTESWAP_H
#define BYTESWAP_H

#include <algorithm>
#include <cstdint>
#include <cstring>

class ByteSwap
{
public:
    template <typename T>
    static T swapBytes(T value)
    {
        T swapped;
        uint8_t *src = reinterpret_cast<uint8_t *>(&value);
        uint8_t *dest = reinterpret_cast<uint8_t *>(&swapped);
        std::reverse_copy(src, src + sizeof(T), dest);
        return swapped;
    }

    template <typename T>
    static void copyAndSwapIfNeeded(T &dest, const uint8_t *src, bool isBigEndian)
    {
        std::memcpy(&dest, src, sizeof(T));
        if (isBigEndian)
        {
            dest = swapBytes(dest);
        }
    }
};

#endif // BYTESWAP_H
