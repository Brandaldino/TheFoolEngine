#include "tfpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "PlatForm/OpenGL/OpenGLBuffer.h"

namespace TheFoolEngine {
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLVertexBuffer>(size);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLVertexBuffer>(vertices, size);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGLIndexBuffer>(size);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI::None is currentlly not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGLIndexBuffer>(indices, size);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}
}
