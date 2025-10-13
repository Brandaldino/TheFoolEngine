#pragma once

#include "ModelImporter.h"

#include "RendererAPI.h"
#include "PerspectiveCameraController.h"
#include "Shader.h"

namespace TheFoolEngine
{

	class MeshRenderer
	{
	public:
		static void Init();
		static void Shutdown();	// no actual function

		static void SetMaterial(const Ref<Material>& material);
		static bool SetSLOTS(int slotPos, TextureType type, const Ref<Texture2D>& texture);
		static void SubmitMeshData(const glm::vec3& position, const glm::vec3& size, uint32_t materialHash, const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices);
		static void SubmitMeshData(const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices, const glm::mat4& transform = glm::mat4(1.0f));
		
		static void AddLight(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, int type = 1);

		static void BeginScene(PerspectiveCamera& camera);
		static void EndScene();

		static void Flush();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t MeshCount = 0;

			uint32_t CurrentVertexCount = 0;
			uint32_t CurrentIndexCount = 0;

			uint32_t GetCurrentDrawCalls() { return DrawCalls; }
			uint32_t GetCurrentMeshCount() { return MeshCount; }

			uint32_t GetCurrentVertexCount() { return CurrentVertexCount; }
			uint32_t GetCurrentIndexCount() { return CurrentIndexCount; }
		};

		static void ResetStats();
		static Statistics& GetStats();
	private:
		static void FlushAndReset();
		static void UploadBatchData();
	};

}
