#include "tfpch.h"
#include "PointShadowMap.h"

#include "Renderer.h"
#include "../../PlatForm/OpenGL/OpenGLPointShadowMap.h"

namespace TheFoolEngine
{
    Ref<PointShadowMap> PointShadowMap::Create(uint32_t size)
    {
        switch (RendererAPI::GetAPI())
        {
        case RendererAPI::API::None:
            TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return CreateRef<OpenGLPointShadowMap>(size);
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }
}