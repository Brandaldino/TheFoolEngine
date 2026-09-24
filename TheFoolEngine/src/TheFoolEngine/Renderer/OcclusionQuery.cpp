#include "tfpch.h"
#include "OcclusionQuery.h"

#include "Renderer.h"

#include "PlatForm/OpenGL/OpenGLOcclusionQuery.h"

namespace TheFoolEngine
{

    Ref<OcclusionQuery> OcclusionQuery::Create()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:	return CreateRef<OpenGLOcclusionQuery>();
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }

}