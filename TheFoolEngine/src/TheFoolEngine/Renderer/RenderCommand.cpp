#include "tfpch.h"
#include "RenderCommand.h"

#include "PlatForm/OpenGL/OpenGLRendererAPI.h"

namespace TheFoolEngine
{

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}
