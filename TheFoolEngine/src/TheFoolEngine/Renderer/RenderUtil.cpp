#include "tfpch.h"
#include "RenderUtil.h"

#include "RendererAPI.h"
#include "PlatForm/OpenGL/OpenGLRenderUtil.h"

namespace TheFoolEngine
{
    RenderUtil* RenderUtil::s_Instance = nullptr;

    RenderUtil* RenderUtil::Get()
    {
        if (!s_Instance)
        {
            switch (RendererAPI::GetAPI())
            {
                case RendererAPI::API::OpenGL:
                    s_Instance = new OpenGLRenderUtil();
                    break;
            }
        }
        return s_Instance;
    }
}
