#pragma once

#include <string>

#include "../Core/Base.h"

namespace TheFoolEngine
{
    class CubeMapBase
    {
    public:
        virtual ~CubeMapBase() = default;

        virtual void Bind(uint32_t slot = 0) const = 0;
        virtual uint32_t GetRendererID() const = 0;

        virtual std::string GetPath() const = 0;
    };

    class CubeMap : public CubeMapBase
    {
    public:
        virtual ~CubeMap() = default;

        static Ref<CubeMap> Create(uint32_t size);
        static Ref<CubeMap> Create(const std::filesystem::path& path);
    };
}