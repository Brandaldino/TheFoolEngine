#pragma once

#include "TheFoolEngine/Renderer/RendererAPI.h"

namespace TheFoolEngine 
{

	class OpenGLRendererAPI :public RendererAPI 
    {
	public:
		virtual void Init() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

        // === Depth State =========================================
        virtual void SetDepthFunc(DepthFunc func) override;
        virtual void SetDepthWrite(DepthWrite flag) override;
        virtual void SetDepthTest(DepthTest flag) override;
        // === Rasterization State =================================
        virtual void SetCullMode(CullMode mode) override;
        virtual void SetPolygonMode(PolygonMode mode) override;
        // === Blend State =========================================
        virtual void SetBlendMode(BlendMode mode) override;
        virtual void SetBlend(bool flag) override;
        // === Draw ================================================
        virtual void DrawArrays(DrawMode mode, uint32_t count) override;
        // =========================================================
	};

}
