#include "tfpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "PlatForm/OpenGL/OpenGLTexture.h"

namespace TheFoolEngine {

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) 
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(width, height);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path) 
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(path);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

    Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(path);
        }

        TF_CORE_ASSERT(false, "Unknown RendererAPI.");
        return nullptr;
    }

	Ref<Texture2D> Texture2D::Create(const void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(data, size);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

}

