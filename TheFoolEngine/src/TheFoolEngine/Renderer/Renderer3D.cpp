#include "tfpch.h"
#include "Renderer3D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine {

	static glm::vec3 LightPos = { 5.0f, 5.0f , 5.0f };

	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor = 1.0f;
		glm::vec3 Normal;
		// TODO: maskid
	};

	struct Renderer3DData {
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;	// quad
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;	// TODO: RenderCaps

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Shader> LightShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr;
		CubeVertex* CubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;	// 0 = white texture

		glm::vec4 QuadVertexPositions[4];

		glm::vec4 CubeVertexPositions[36];

		Renderer3D::Statistics Stats;
	};

	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		s_Data.CubeVertexArray = VertexArray::Create();
		s_Data.CubeVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CubeVertex));
		s_Data.CubeVertexBuffer->SetLayout(
			{
				{ShaderDataType::Float3,	"a_Position"		},
				{ShaderDataType::Float4,	"a_Color"			},
				{ShaderDataType::Float2,	"a_TexCoord"		},
				{ShaderDataType::Float,		"a_TexIndex"		},
				{ShaderDataType::Float,		"a_TilingFactor"	},
				{ShaderDataType::Float3,	"a_Normal"			}
			}
		);

		s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

		s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.CubeVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t texData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&texData, sizeof(uint32_t));

		int32_t samplers[s_Data.MaxTextureSlots];
		for (int32_t i = 0; i < s_Data.MaxTextureSlots; ++i)
			samplers[i] = i;

		s_Data.TextureShader = Shader::Create("assets/shaders/Texture3D.glsl");
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.LightShader = Shader::Create("assets/shaders/Light.glsl");
		s_Data.LightShader->Bind();

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		// Local Coordinates - Quad
		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f,1.0f };
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f,1.0f };
		s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f,1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f,1.0f };

		// Local Coordinates - Cube
		s_Data.CubeVertexPositions[0] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[1] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[2] = { 0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[3] = { 0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[4] = { -0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[5] = { -0.5f, -0.5f, -0.5f, 1.0f };

		s_Data.CubeVertexPositions[6] = { -0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[7] = { 0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[8] = { 0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[9] = { 0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[10] = { -0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[11] = { -0.5f, -0.5f, 0.5f, 1.0f };

		s_Data.CubeVertexPositions[12] = { -0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[13] = { -0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[14] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[15] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[16] = { -0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[17] = { -0.5f, 0.5f, 0.5f, 1.0f };

		s_Data.CubeVertexPositions[18] = { 0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[19] = { 0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[20] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[21] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[22] = { 0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[23] = { 0.5f, 0.5f, 0.5f, 1.0f };

		s_Data.CubeVertexPositions[24] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[25] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[26] = { 0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[27] = { 0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[28] = { -0.5f, -0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[29] = { -0.5f, -0.5f, -0.5f, 1.0f };

		s_Data.CubeVertexPositions[30] = { -0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[31] = { 0.5f, 0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[32] = { 0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[33] = { 0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[34] = { -0.5f, 0.5f, 0.5f, 1.0f };
		s_Data.CubeVertexPositions[35] = { -0.5f, 0.5f, -0.5f, 1.0f };
	}

	void Renderer3D::Shutdown()
	{

	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		s_Data.TextureShader->Bind();

		s_Data.TextureShader->SetMat4("u_View", camera.GetViewMatrix());
		s_Data.TextureShader->SetMat4("u_Projection", camera.GetProjectionMatrix());

		s_Data.TextureShader->SetFloat3("u_CameraPos", camera.GetPosition());

		// Materials
		s_Data.TextureShader->SetFloat3("u_Material.ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_Material.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		s_Data.TextureShader->SetFloat3("u_Material.specular", glm::vec3(0.5f));
		s_Data.TextureShader->SetFloat("u_Material.shininess", 32.0f);

		// Light
		glm::vec3 lightColor = glm::vec3(1.0f);

		s_Data.TextureShader->SetFloat4("u_LightSourceColor", { lightColor , 1.0f });
		s_Data.TextureShader->SetFloat3("u_PointLight.position", LightPos);

		// Light Materials
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.9f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

		s_Data.TextureShader->SetFloat3("u_PointLight.specular", glm::vec3(1.0f));
		s_Data.TextureShader->SetFloat3("u_PointLight.diffuse", diffuseColor);
		s_Data.TextureShader->SetFloat3("u_PointLight.ambient", ambientColor);

		// Attenuation
		s_Data.TextureShader->SetFloat("u_PointLight.constant", 0.0f);
		s_Data.TextureShader->SetFloat("u_PointLight.linear", 0.09f);
		s_Data.TextureShader->SetFloat("u_PointLight.quadratic", 0.0032f);

		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera, bool value)
	{
		s_Data.LightShader->Bind();

		s_Data.LightShader->SetMat4("u_View", camera.GetViewMatrix());
		s_Data.LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix());

		s_Data.LightShader->SetFloat4("u_LightSourceColor", glm::vec4(1.0f));

		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera, const glm::vec3& lightPos)
	{
		s_Data.TextureShader->Bind();

		s_Data.TextureShader->SetMat4("u_View", camera.GetViewMatrix());
		s_Data.TextureShader->SetMat4("u_Projection", camera.GetProjectionMatrix());

		s_Data.TextureShader->SetFloat3("u_CameraPos", camera.GetPosition());

		// Materials
		s_Data.TextureShader->SetFloat3("u_Material.ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_Material.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		s_Data.TextureShader->SetFloat3("u_Material.specular", glm::vec3(0.5f));
		s_Data.TextureShader->SetFloat("u_Material.shininess", 32.0f);

		// Light
		glm::vec3 lightColor = glm::vec3(1.0f);
		s_Data.TextureShader->SetFloat4("u_LightSourceColor", { lightColor , 1.0f });
		// PointLight
		// PointLight Position
		glm::vec3 pointLightPos[] = {
			glm::vec3(-5.0f,	5.0f,	-5.0f),
			glm::vec3(5.0F,		5.0f,	5.0f),
			glm::vec3(5.0f,		5.0f,	-5.0f),
			glm::vec3(-5.0f,	5.0f,	5.0f)
		};

		// point light 1
		s_Data.TextureShader->SetFloat3("u_PointLight[0].position", pointLightPos[0]);
		s_Data.TextureShader->SetFloat3("u_PointLight[0].ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_PointLight[0].diffuse", { 0.8f, 0.8f, 0.8f });
		s_Data.TextureShader->SetFloat3("u_PointLight[0].specular", { 0.8f, 0.8f, 0.8f });
		s_Data.TextureShader->SetFloat("u_PointLight[0].constant", 1.0f);
		s_Data.TextureShader->SetFloat("u_PointLight[0].linear", 0.09f);
		s_Data.TextureShader->SetFloat("u_PointLight[0].quadratic", 0.032f);

		// point light 2
		s_Data.TextureShader->SetFloat3("u_PointLight[1].position", pointLightPos[1]);
		s_Data.TextureShader->SetFloat3("u_PointLight[1].ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_PointLight[1].diffuse", { 0.8f, 0.8f, 0.8f });
		s_Data.TextureShader->SetFloat3("u_PointLight[1].specular", { 1.0f, 1.0f, 1.0f });
		s_Data.TextureShader->SetFloat("u_PointLight[1].constant", 1.0f);
		s_Data.TextureShader->SetFloat("u_PointLight[1].linear", 0.09f);
		s_Data.TextureShader->SetFloat("u_PointLight[1].quadratic", 0.032f);
		// point light 3
		s_Data.TextureShader->SetFloat3("u_PointLight[2].position", pointLightPos[2]);
		s_Data.TextureShader->SetFloat3("u_PointLight[2].ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_PointLight[2].diffuse", { 0.8f, 0.8f, 0.8f });
		s_Data.TextureShader->SetFloat3("u_PointLight[2].specular", { 1.0f, 1.0f, 1.0f });
		s_Data.TextureShader->SetFloat("u_PointLight[2].constant", 1.0f);
		s_Data.TextureShader->SetFloat("u_PointLight[2].linear", 0.09f);
		s_Data.TextureShader->SetFloat("u_PointLight[2].quadratic", 0.032f);
		// point light 4
		s_Data.TextureShader->SetFloat3("u_PointLight[3].position", pointLightPos[3]);
		s_Data.TextureShader->SetFloat3("u_PointLight[3].ambient", glm::vec3(0.2f));
		s_Data.TextureShader->SetFloat3("u_PointLight[3].diffuse", { 0.8f, 0.8f, 0.8f });
		s_Data.TextureShader->SetFloat3("u_PointLight[3].specular", { 1.0f, 1.0f, 1.0f });
		s_Data.TextureShader->SetFloat("u_PointLight[3].constant", 1.0f);
		s_Data.TextureShader->SetFloat("u_PointLight[3].linear", 0.09f);
		s_Data.TextureShader->SetFloat("u_PointLight[3].quadratic", 0.032f);

		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::EndScene()
	{
		uint32_t dataSize = (uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase;
		s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer3D::Flush()
	{
		// Bind textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
			s_Data.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer3D::FlushAndReset()
	{
		EndScene();

		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec3& size, const glm::vec4& color) {
		TF_PROFILE_FUNCTION();

		DrawQuad({ position.x, position.y, 0.0f}, size, color);
	}

	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color) {

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), size);

		DrawQuad(transform, color);
	}

	void Renderer3D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, const glm::vec3& normal)
	{
		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
			FlushAndReset();

		const float textureIndex = 0.0f;	// White Texture
		const float tilingFactor = 1.0f;

		const  glm::vec2 texCoord[4] = { { 0.0f,0.0f } , { 1.0f,0.0f } , { 1.0f,1.0f } , { 0.0f,1.0f } };

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
		glm::vec3 normalRes = glm::normalize(normalMatrix * normal);

		for (std::size_t i = 0; i < 4; ++i)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = texCoord[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->Normal = normalRes;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 6;

		s_Data.Stats.QuadCount++;
		
	}

	void Renderer3D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{

	}

	void Renderer3D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec3& normal, float tilingFactor, const glm::vec4& tintColor)
	{
		constexpr glm::vec4 color = { 1.0f,1.0f, 1.0f, 1.0f };
		constexpr std::size_t quadVertexCount = 4;
		constexpr glm::vec2 texCoords[] = { {0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f},{0.0f,1.0f} };

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
				FlushAndReset();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
		glm::vec3 normalRes = glm::normalize(normalMatrix * normal);

		for (std::size_t i = 0; i < quadVertexCount; ++i)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.CubeVertexBufferPtr->Normal = normalRes;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 6;

		s_Data.Stats.QuadCount++;

	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const std::array<glm::vec4, 6>& color)
	{
		DrawCube(position, size, color, glm::vec3(0.0f));
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& singlecolor)
	{
		std::array<glm::vec4, 6> color;
		color.fill(singlecolor);

		DrawCube(position, size, color);
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const std::array<glm::vec4, 6>& color, const glm::vec3& radians)
	{
		glm::mat4 cubeRotation = glm::rotate(glm::mat4(1.0f), glm::radians(radians.x), glm::vec3(0.0f, 1.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(radians.y), glm::vec3(1.0f, 0.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(radians.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 normal(0.0f, 0.0f, -1.0f);
		// Front Face
		glm::vec3 frontnormal = { 0.0f, 0.0f, -0.5f };
		glm::mat4 frontTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), frontnormal);
		DrawQuad(frontTransform, color[0], glm::normalize(normal));

		// Back Face
		glm::vec3 backnormal = { 0.0f, 0.0f, 0.5f };
		glm::mat4 backTransform = 
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation 
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), backnormal)
			* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(backTransform, color[1], glm::normalize(normal));

		// Left Face
		glm::vec3 leftface = { -0.5f, 0.0f, 0.0f };
		glm::mat4 leftTransform = 
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation 
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), leftface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(leftTransform, color[2], glm::normalize(normal));

		// Right Face
		glm::vec3 rightface = { 0.5f, 0.0f, 0.0f };
		glm::mat4 rightTransform = 
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation 
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), rightface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(rightTransform, color[3], glm::normalize(normal));

		// Bottom Face
		glm::vec3 bottomface = { 0.0f, -0.5f, 0.0f };
		glm::mat4 bottomTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), bottomface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawQuad(bottomTransform, color[4], glm::normalize(normal));

		// Top Face
		glm::vec3 topface = { 0.0f, 0.5f, 0.0f };
		glm::mat4 topTransform = 
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation 
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), topface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawQuad(topTransform, color[5], glm::normalize(normal));
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& singlecolor, const glm::vec3& radians)
	{
		std::array<glm::vec4, 6> color;
		color.fill(singlecolor);
		DrawCube(position, size, color, radians);
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec3& radians)
	{
		glm::mat4 cubeRotation = glm::rotate(glm::mat4(1.0f), glm::radians(radians.x), glm::vec3(0.0f, 1.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(radians.y), glm::vec3(1.0f, 0.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(radians.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 normal(0.0f, 0.0f, -1.0f);
		// Front Face
		glm::vec3 frontnormal = { 0.0f, 0.0f, -0.5f };
		glm::mat4 frontTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), frontnormal);
		DrawQuad(frontTransform, texture, glm::normalize(normal));

		// Back Face
		glm::vec3 backnormal = { 0.0f, 0.0f, 0.5f };
		glm::mat4 backTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), backnormal)
			* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(backTransform, texture, glm::normalize(normal));

		// Left Face
		glm::vec3 leftface = { -0.5f, 0.0f, 0.0f };
		glm::mat4 leftTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), leftface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(leftTransform, texture, glm::normalize(normal));

		// Right Face
		glm::vec3 rightface = { 0.5f, 0.0f, 0.0f };
		glm::mat4 rightTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), rightface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawQuad(rightTransform, texture, glm::normalize(normal));

		// Bottom Face
		glm::vec3 bottomface = { 0.0f, -0.5f, 0.0f };
		glm::mat4 bottomTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), bottomface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawQuad(bottomTransform, texture, glm::normalize(normal));

		// Top Face
		glm::vec3 topface = { 0.0f, 0.5f, 0.0f };
		glm::mat4 topTransform =
			glm::translate(glm::mat4(1.0f), position)
			* cubeRotation
			* glm::scale(glm::mat4(1.0f), size)
			* glm::translate(glm::mat4(1.0f), topface)
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawQuad(topTransform, texture, glm::normalize(normal));
	}

	void Renderer3D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics& Renderer3D::GetStats()
	{
		return s_Data.Stats;
	}
}
