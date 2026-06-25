#include "tfpch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "PlatForm/OpenGL/OpenGLVertexArray.h"

namespace TheFoolEngine{

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLVertexArray>();
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}
}
