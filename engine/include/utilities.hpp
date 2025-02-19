#ifndef UTILITIES_H
#define UTILITIES_H

namespace lum::utils
{
    constexpr uint32_t HashStr32(const char *str, uint32_t value = 0x811C9DC5)
    {
        if (*str == '\0')
        {
            return value;
        }
        else
        {
            return HashStr32(str + 1, (value ^ uint32_t(*str)) * 0x01000193);
        }
    }

    constexpr uint64_t HashStr64(const char *str, uint64_t value = 0xCBF29CE484222325)
    {
        if (*str == '\0')
        {
            return value;
        }
        else
        {
            return HashStr64(str + 1, (value ^ uint64_t(*str)) * 0x100000001B3);
        }
    }
}

#endif // !UTILITIES_H
