#pragma once

#include <cstdint>
#include <random>

namespace TheFoolEngine
{
    using UUID = uint64_t;

    inline UUID GenerateUUID()
    {
        static std::mt19937_64 rng(std::random_device{}());
        return rng();
    }
}