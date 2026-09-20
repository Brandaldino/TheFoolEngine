#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

namespace TheFoolEngine
{
    namespace HashUtils
    {
        template<typename Type>
        inline void HashCombine(std::size_t& seed, const Type& value)
        {
            seed ^= std::hash<Type>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        template<typename... Fields>
        uint64_t HashFields(const Fields&... fields)
        {
            std::size_t seed = 0;
            (HashCombine<Fields>(seed, fields), ...);
            return static_cast<uint64_t>(seed);
        }
    }
}