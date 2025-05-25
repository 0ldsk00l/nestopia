#pragma once

#include <cstdint>
#include <string>

class Hasher {
public:
    static uint32_t crc(void *data, size_t size);
    static std::string md5(void *data, size_t size);
};
