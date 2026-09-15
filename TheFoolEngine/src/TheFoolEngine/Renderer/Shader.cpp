#include "tfpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "PlatForm/OpenGL/OpenGLShader.h"

namespace TheFoolEngine 
{
    // ===== Shader ======================================
	TheFoolEngine::Ref<Shader> Shader::Create(const std::string& filepath) 
    {
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLShader>(filepath);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

	TheFoolEngine::Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:		TF_CORE_ASSERT(false, "RendererAPI:: None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:		return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		TF_CORE_ASSERT(false, "Unknown RendererAPI.");
		return nullptr;
	}

    // ===== ShaderLibrary ======================================
    ShaderLibrary& ShaderLibrary::Get()
    {
        static ShaderLibrary instance; // C++11
        return instance;
    }

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader) 
    {
		TF_CORE_ASSERT(!Exists(name), "Shader already exists.");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader) 
    {
		auto& name = shader->GetName();
		Add(name, shader);
	}

	TheFoolEngine::Ref<Shader> ShaderLibrary::Load(const std::string& filepath) 
    {
        // Extract the filename as the key (matching OpenGLShader::m_Name: remove path and extension)
        auto lastSlash = filepath.find_last_of("/\\");
        std::string name = filepath.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
        auto lastDot = name.find_last_of('.');
        name = name.substr(0, lastDot);

        if (Exists(name))
            return m_Shaders[name]; // Cache hit, skip recompilation

		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	TheFoolEngine::Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath) 
    {
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	TheFoolEngine::Ref<Shader> ShaderLibrary::Get(const std::string& name) 
    {
		TF_CORE_ASSERT(Exists(name), "Shader is not exists.");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const 
    {
		return m_Shaders.find(name) != m_Shaders.end();
	}

}
