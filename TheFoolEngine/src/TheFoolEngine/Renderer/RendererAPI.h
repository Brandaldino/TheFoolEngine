#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>

namespace TheFoolEngine{

	class RendererAPI {
	public:
        // === API =======================
		enum class API
		{
			None = 0, OpenGL = 1
		};

        // === Render State ==============
        enum class DepthFunc    : uint8_t { Less, LessEqual };
        enum class DepthWrite   : uint8_t { On, Off };
        enum class DepthTest    : uint8_t { On, Off };
        enum class CullMode     : uint8_t { None, Front, Back };
        enum class BlendMode    : uint8_t { None, Alpha, Additive };
        enum class DrawMode     : uint8_t { Triangles };
        enum class PolygonMode  : uint8_t { Fill, Line };
	public:
		virtual void Init() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0; 
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

        // === Depth State =========================================
        virtual void SetDepthFunc(DepthFunc func) = 0;
        virtual void SetDepthWrite(DepthWrite flag) = 0;
        virtual void SetDepthTest(DepthTest flag) = 0;
        // === Rasterization State =================================
        virtual void SetCullMode(CullMode mode) = 0;
        virtual void SetPolygonMode(PolygonMode mode) = 0;
        // === Blend State =========================================
        virtual void SetBlendMode(BlendMode mode) = 0;
        virtual void SetBlend(bool flag) = 0;
        // === Draw ================================================
        virtual void DrawArrays(DrawMode mode, uint32_t count) = 0;
        // =========================================================
		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

}
