#pragma once

#include "../../TheFoolEngine/Renderer/CubeMap.h"

#include <glad/glad.h>

namespace TheFoolEngine
{
    class OpenGLCubeMap : public CubeMap
    {
    public:
        OpenGLCubeMap(uint32_t size);
        OpenGLCubeMap(const std::filesystem::path& path);
        virtual ~OpenGLCubeMap();

        virtual void Bind(uint32_t slot) const override;
        virtual uint32_t GetRendererID() const override { return m_RendererID; }

        virtual std::string GetPath() const override { return m_Path; };

        virtual void GenerateMipmap() override;
    private:
        std::string m_Path;
        uint32_t m_RendererID;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////

    class CubeMapCache
    {
    public:
        static bool Regist(const std::filesystem::path& path, Ref<CubeMap> cubemap);
        static Ref<CubeMap> FindCubeMap(const std::filesystem::path& path);
        static void Clear();
    private:
        CubeMapCache() = delete;
    private:
        static std::unordered_map<std::filesystem::path, Ref<CubeMap>> s_CubeMapCache;
    };
}