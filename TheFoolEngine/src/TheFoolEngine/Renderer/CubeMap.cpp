#include "tfpch.h"
#include "CubeMap.h"

#include "Renderer.h"
#include "../../PlatForm/OpenGL/OpenGLCubeMap.h"

namespace TheFoolEngine
{
    Ref<CubeMap> CubeMap::Create(uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:            TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:          return CreateRef<OpenGLCubeMap>(size);
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }

    Ref<CubeMap> CubeMap::Create(const std::filesystem::path& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:            TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:
            {
                auto cache = CubeMapCache::FindCubeMap(path);
                if (cache)
                    return cache;
                auto cubemap = CreateRef<OpenGLCubeMap>(path);
                CubeMapCache::Regist(path, cubemap);
                return cubemap;
            }
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }

}
