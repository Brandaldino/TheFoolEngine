#include "tfpch.h"
#include "PointShadowMap.h"

#include "Renderer.h"
#include "../../PlatForm/OpenGL/OpenGLPointShadowMap.h"

namespace TheFoolEngine
{
    Ref<PointShadowMap> PointShadowMap::Create(uint32_t size, uint32_t layerCount)
    {
        switch (RendererAPI::GetAPI())
        {
        case RendererAPI::API::None:
            TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return CreateRef<OpenGLPointShadowMap>(size, layerCount);
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }
}