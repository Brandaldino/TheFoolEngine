#pragma once

#include <TheFoolEngine.h>

class ExampleLayer :public TheFoolEngine::Layer {
public:
	ExampleLayer()
		:Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		m_VertexArray = TheFoolEngine::VertexArray::Create();
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f,0.2f,0.8f,1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f,0.3f,0.8f,1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f,0.8f,0.2f,1.0f
		};

		TheFoolEngine::BufferLayout layout = {
			{ TheFoolEngine::ShaderDataType::Float3,"a_Position"},
			{ TheFoolEngine::ShaderDataType::Float4,"a_Color"},
		};
		TheFoolEngine::Ref<TheFoolEngine::VertexBuffer> trangleVB = TheFoolEngine::VertexBuffer::Create(vertices, sizeof(vertices));
		trangleVB->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(trangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		TheFoolEngine::Ref<TheFoolEngine::IndexBuffer> trangleIB = TheFoolEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(trangleIB);

		m_SquareVA = TheFoolEngine::VertexArray::Create();
		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		TheFoolEngine::Ref<TheFoolEngine::VertexBuffer> squareVB = TheFoolEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
				{ TheFoolEngine::ShaderDataType::Float3,"a_Position"},
				{ TheFoolEngine::ShaderDataType::Float2,"a_TexCoord"}
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		TheFoolEngine::Ref<TheFoolEngine::IndexBuffer> squareIB = TheFoolEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			
			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader = TheFoolEngine::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;			
			uniform vec3 u_LightDir;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader = TheFoolEngine::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_shaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = TheFoolEngine::Texture2D::Create("assets/textures/Checkerboard.png");
		m_TextLogoTexture = TheFoolEngine::Texture2D::Create("assets/textures/Skadi.png");
		// m_TextLogoTexture = TheFoolEngine::Texture2D::Create("assets/textures/testBlending.png");	// Transparency test

		std::dynamic_pointer_cast<TheFoolEngine::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<TheFoolEngine::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(TheFoolEngine::TimeStep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		TheFoolEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		TheFoolEngine::RenderCommand::Clear();

		TheFoolEngine::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<TheFoolEngine::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<TheFoolEngine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; ++y) {
			for (int x = 0; x < 20; ++x) {
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				TheFoolEngine::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_shaderLibrary.Get("Texture");

		m_Texture->Bind();
		TheFoolEngine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_TextLogoTexture->Bind();
		TheFoolEngine::Renderer::Submit(textureShader, m_SquareVA,
			glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, -0.25, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// Triangle
		// TheFoolEngine::Renderer::Submit(m_Shader, m_VertexArray);

		TheFoolEngine::Renderer::EndScene();

	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(TheFoolEngine::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}
private:
	TheFoolEngine::ShaderLibrary m_shaderLibrary;
	TheFoolEngine::Ref<TheFoolEngine::Shader> m_Shader;
	TheFoolEngine::Ref<TheFoolEngine::VertexArray> m_VertexArray;

	TheFoolEngine::Ref<TheFoolEngine::Shader> m_FlatColorShader;
	TheFoolEngine::Ref<TheFoolEngine::VertexArray> m_SquareVA;

	TheFoolEngine::Ref<TheFoolEngine::Texture2D> m_Texture, m_TextLogoTexture;

	TheFoolEngine::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};