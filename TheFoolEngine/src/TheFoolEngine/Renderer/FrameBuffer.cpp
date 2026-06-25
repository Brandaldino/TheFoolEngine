#include "tfpch.h"
#include "FrameBuffer.h"

#include "TheFoolEngine/Renderer/Renderer.h"

#include "PlatForm/OpenGL/OpenGLFrameBuffer.h"

namespace TheFoolEngine {

	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLFrameBuffer>(spec);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

}
