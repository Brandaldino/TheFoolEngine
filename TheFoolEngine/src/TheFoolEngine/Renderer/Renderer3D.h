#pragma once

#include "PerspectiveCameraController.h"

#include "Texture.h"

#include <array>

namespace TheFoolEngine {

	class Renderer3D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const PerspectiveCamera& camera);
		static void BeginScene(const PerspectiveCamera& camera, bool value);
		static void BeginScene(const PerspectiveCamera& camera, const glm::vec3& lightPosOrlightDir);

		static void EndScene();

		static void Flush();

		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec3& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, const glm::vec3& normal);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec3& normal, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const std::array<glm::vec4,6>& color);
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& singlecolor);
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const std::array<glm::vec4, 6>& color, const glm::vec3& radians);
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& singlecolor, const glm::vec3& radians);
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec3& radians);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics& GetStats();
	private:
		static void FlushAndReset();
	};

}
