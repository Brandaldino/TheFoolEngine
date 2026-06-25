#include "tfpch.h"
#include "MeshRenderer.h"

#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{
	struct Light {
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
		int type; 			// 0 = Parallel light, 1 = Point source light
	};

	struct BatchMeshRenderer
	{
		static const uint32_t MaxVertexCount = 1000 * 1000 * 3;
		static const uint32_t MaxIndexCount = 1000 * 1000 * 9;
		static const uint32_t MaxTextureSlots = 32;

		uint32_t CurVertexCount = 0;
		uint32_t CurIndexCount = 0;

		// material and texture
		Ref<Material> CurrentMaterial;
		uint32_t CurrentMaterialID = 0;
		Ref<Texture2D> WhiteTexture;
		uint32_t TextureSlotIndex = 1;	// 0 = white texture
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		std::array<TextureType, MaxTextureSlots> SLOTS;

		// vertex buffer ptr
		MeshVertexData* MeshVertexDataBufferBase = nullptr;
		MeshVertexData* MeshVertexDataBufferPtr = nullptr;
		// index buffer ptr
		uint32_t* MeshIndexDataBufferBase = nullptr;
		uint32_t* MeshIndexDataBufferPtr = nullptr;

		// GPU source
		Ref<VertexArray> BatchMeshVertexArray;
		Ref<VertexBuffer> BatchMeshVertexBuffer;
		Ref<IndexBuffer> BatchMeshIndexBuffer;

		// Shader
		Ref<Shader> MeshShader;
		Ref<Shader> LightShader;

		// Camera and Transform

		// Lights
		std::vector<Light> Lights;
		int LightCount = 0;

		// Ambient Light
		glm::vec3 AmbientLight{ 0.1f, 0.1f, 0.1f };

		// Stats
		MeshRenderer::Statistics Stats;
	};

	static BatchMeshRenderer s_Data;

	void MeshRenderer::Init()
	{
		// create vertex buffer | array
		s_Data.BatchMeshVertexArray = VertexArray::Create();
		s_Data.BatchMeshVertexBuffer = VertexBuffer::Create(s_Data.MaxVertexCount * sizeof(MeshVertexData));

		// set layout
		s_Data.BatchMeshVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3,	"a_Position"	},
				{ ShaderDataType::Float3,	"a_Normal"		},
				{ ShaderDataType::Float2,	"a_TexCoords"	},
				{ ShaderDataType::Float3,	"a_Tangent"		},
				{ ShaderDataType::Float3,	"a_Bitangent"	},
				{ ShaderDataType::Int4,		"a_BoneIDs"		},
				{ ShaderDataType::Float4,	"a_Weights"		}
			}
		);

		s_Data.BatchMeshVertexArray->AddVertexBuffer(s_Data.BatchMeshVertexBuffer);

		// buffer init
		s_Data.MeshVertexDataBufferBase = new MeshVertexData[s_Data.MaxVertexCount];
		s_Data.MeshIndexDataBufferBase = new uint32_t[s_Data.MaxIndexCount];
		TF_CORE_ASSERT(s_Data.MeshVertexDataBufferBase || s_Data.MeshIndexDataBufferBase, "Batch Renderer Buffer Init Failed.");

		s_Data.MeshVertexDataBufferPtr= s_Data.MeshVertexDataBufferBase;
		s_Data.MeshIndexDataBufferPtr = s_Data.MeshIndexDataBufferBase;

		// index
		s_Data.BatchMeshIndexBuffer = IndexBuffer::Create(s_Data.MaxIndexCount * sizeof(uint32_t));
		s_Data.BatchMeshVertexArray->SetIndexBuffer(s_Data.BatchMeshIndexBuffer);

		// white texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTexData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTexData, sizeof(uint32_t));

		// texture slots
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; ++i)
			samplers[i] = i;

		// shader init
		s_Data.MeshShader = Shader::Create("assets/shaders/PBRLightShader.glsl");
		s_Data.MeshShader->Bind();
		s_Data.MeshShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.LightShader = Shader::Create("assets/shaders/Lights.glsl");
		s_Data.LightShader->Bind();

		// set each face all texture slots to 0
		// s_Data.TextureSlots.fill(s_Data.WhiteTexture);
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		// stats init
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.MeshCount = 0;
		s_Data.Stats.CurrentVertexCount = 0;
		s_Data.Stats.CurrentIndexCount = 0;
	}

	void MeshRenderer::Shutdown()
	{
		delete[] s_Data.MeshVertexDataBufferBase;
		delete[] s_Data.MeshIndexDataBufferBase;
		s_Data.MeshVertexDataBufferBase = nullptr;
		s_Data.MeshIndexDataBufferBase = nullptr;
		s_Data.MeshVertexDataBufferPtr = nullptr;
		s_Data.MeshIndexDataBufferPtr = nullptr;
	}

	void MeshRenderer::SetMaterial(const Ref<Material>& material)
	{
		s_Data.CurrentMaterial = material;
		s_Data.CurrentMaterialID = MaterialManager::Get().GetIDByHash(material->GetHash());

		auto hash = material->GetHash();

		s_Data.SLOTS =
		{
			// Core texture type (fixed position)
			TextureType::WhiteTexture,  // 0 - Default fallback
			TextureType::Diffuse,       // 1 - Basic color
			TextureType::Normal,        // 2 - Normal
			TextureType::Specular,      // 3 - Specular
			TextureType::Emissive,      // 4 - Emissive
			TextureType::Opacity,       // 5 - Opacity
			TextureType::Height,        // 6 - Height

			// Optional texture types
			TextureType::Ambient,       // 9
			TextureType::Shininess,     // 10
			TextureType::Displacement,  // 11
			TextureType::Lightmap,      // 7
			TextureType::Reflection,    // 8
		};

		s_Data.TextureSlotIndex = 12;

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
		{
			s_Data.TextureSlots[i] = material->GetTextureByType(s_Data.SLOTS[i]).size() != 0 ? 
				material->GetTextureByType(s_Data.SLOTS[i])[0] : s_Data.WhiteTexture;

			if (s_Data.TextureSlots[i] == s_Data.WhiteTexture)
				int temp = 0;
		}
	}

	void MeshRenderer::SubmitMeshData(const glm::vec3& position, const glm::vec3& size, uint32_t materialHash, const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices)
	{
		TF_PROFILE_FUNCTION();
		// check whether it will exceed the buffer limit.
		if (s_Data.CurVertexCount + vertices.size() > s_Data.MaxVertexCount ||
			s_Data.CurIndexCount + indices.size() > s_Data.MaxIndexCount)
			FlushAndReset();

		auto checkID = MaterialManager::Get().GetIDByHash(materialHash);
		if (checkID != s_Data.CurrentMaterialID)
		{
			FlushAndReset();

			// get new material and init
			auto nextMaterial = MaterialManager::Get().GetMaterialByHash(materialHash);
			if (nextMaterial)
				SetMaterial(nextMaterial);
			else
			{
				TF_CORE_ERROR("Batch Renderer: Failed to find material with hash {0}", materialHash);
				return;
			}
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,size.z });
			
		SubmitMeshData(vertices, indices, transform);
	}

	void MeshRenderer::SubmitMeshData(const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices, const glm::mat4& transform)
	{
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
		uint32_t offset = s_Data.CurVertexCount;

		for (uint32_t i = 0; i < vertices.size(); ++i)
		{
			s_Data.MeshVertexDataBufferPtr->Position = transform * glm::vec4(vertices[i].Position, 1.0f);
			s_Data.MeshVertexDataBufferPtr->Normal = normalMatrix * vertices[i].Normal;
			s_Data.MeshVertexDataBufferPtr->TexCoords = vertices[i].TexCoords;
			s_Data.MeshVertexDataBufferPtr->Tangent = normalMatrix * vertices[i].Tangent;
			s_Data.MeshVertexDataBufferPtr->Bitangent = normalMatrix * vertices[i].Bitangent;
			memcpy(s_Data.MeshVertexDataBufferPtr->BoneIDs, vertices[i].BoneIDs, sizeof(int) * MAX_BONE_INFLUENCE);
			memcpy(s_Data.MeshVertexDataBufferPtr->Weights, vertices[i].Weights, sizeof(float) * MAX_BONE_INFLUENCE);
			s_Data.MeshVertexDataBufferPtr++;
		}

		for (uint32_t i = 0; i < indices.size(); ++i)
		{
			*s_Data.MeshIndexDataBufferPtr = indices[i] + offset;
			s_Data.MeshIndexDataBufferPtr++;
		}

		s_Data.CurVertexCount += vertices.size();
		s_Data.CurIndexCount += indices.size();

		s_Data.Stats.MeshCount++;
		s_Data.Stats.CurrentVertexCount = s_Data.CurVertexCount;
		s_Data.Stats.CurrentIndexCount = s_Data.CurIndexCount;
	}

	void MeshRenderer::AddLight(const glm::vec3& position, const glm::vec3& color, float intensity, int type)
	{
		Light light = 
		{
			position,
			color,
			intensity,
			type
		};

		s_Data.Lights.push_back(light);
		s_Data.LightCount++;
	}

	void MeshRenderer::BeginScene(PerspectiveCamera& camera)
	{
		// Lights
		s_Data.Lights.clear();
		s_Data.LightCount = 0;

		s_Data.MeshShader->Bind();
		s_Data.MeshShader->SetMat4("u_View", camera.GetViewMatrix());
		s_Data.MeshShader->SetMat4("u_Projection", camera.GetProjectionMatrix());
		s_Data.MeshShader->SetFloat3("u_CameraPos", camera.GetPosition());

		s_Data.CurVertexCount = 0;
		s_Data.CurIndexCount = 0;

		s_Data.MeshVertexDataBufferPtr = s_Data.MeshVertexDataBufferBase;
		s_Data.MeshIndexDataBufferPtr = s_Data.MeshIndexDataBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void MeshRenderer::EndScene()
	{
		UploadBatchData();;
		Flush();
	}

	void MeshRenderer::Flush()
	{
		// Add Light
		for (int i = 0; i < s_Data.LightCount; ++i)
		{
			std::string base = "u_Lights[" + std::to_string(i) + "]";
			s_Data.MeshShader->SetFloat3(base + ".position", s_Data.Lights[i].position);
			s_Data.MeshShader->SetFloat3(base + ".color", s_Data.Lights[i].color);
			s_Data.MeshShader->SetFloat(base + ".intensity", s_Data.Lights[i].intensity);
			s_Data.MeshShader->SetInt(base + ".type", s_Data.Lights[i].type);
		}
		s_Data.MeshShader->SetInt("u_LightCount", s_Data.LightCount);

		// Bind Textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i)
			s_Data.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.BatchMeshVertexArray, s_Data.CurIndexCount);

		s_Data.Stats.DrawCalls++;
	}

	void MeshRenderer::FlushAndReset()
	{
		EndScene();

		s_Data.CurVertexCount = 0;
		s_Data.CurIndexCount = 0;
		s_Data.MeshVertexDataBufferPtr = s_Data.MeshVertexDataBufferBase;
		s_Data.MeshIndexDataBufferPtr = s_Data.MeshIndexDataBufferBase;
	}

	void MeshRenderer::UploadBatchData()
	{
		uint32_t vertexDatasize = (uint8_t*)s_Data.MeshVertexDataBufferPtr - (uint8_t*)s_Data.MeshVertexDataBufferBase;
		uint32_t indexDatasize = (uint8_t*)s_Data.MeshIndexDataBufferPtr - (uint8_t*)s_Data.MeshIndexDataBufferBase;

		s_Data.BatchMeshVertexBuffer->SetData(s_Data.MeshVertexDataBufferBase, vertexDatasize);
		s_Data.BatchMeshIndexBuffer->SetData(s_Data.MeshIndexDataBufferBase, indexDatasize);
	}

	void MeshRenderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	MeshRenderer::Statistics& MeshRenderer::GetStats()
	{
		return s_Data.Stats;
	}

}
