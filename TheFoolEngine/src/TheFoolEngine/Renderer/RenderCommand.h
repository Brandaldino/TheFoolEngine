#pragma once

#include "RendererAPI.h"

namespace TheFoolEngine 
{

	class RenderCommand 
    {
	public:
		inline static void Init() 
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color) 
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) 
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void Clear() 
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

        // === Render State ============================================================
        inline static void SetDepthFunc(RendererAPI::DepthFunc func) { s_RendererAPI->SetDepthFunc(func); };
        inline static void SetDepthWrite(RendererAPI::DepthWrite flag) { s_RendererAPI->SetDepthWrite(flag); };
        inline static void SetDepthTest(RendererAPI::DepthTest flag) { s_RendererAPI->SetDepthTest(flag); };
        inline static void SetCullMode(RendererAPI::CullMode mode) { s_RendererAPI->SetCullMode(mode); };
        inline static void SetPolygonMode(RendererAPI::PolygonMode mode) { s_RendererAPI->SetPolygonMode(mode); };
        inline static void SetBlendMode(RendererAPI::BlendMode mode) { s_RendererAPI->SetBlendMode(mode); };
        inline static void SetBlend(bool flag) { s_RendererAPI->SetBlend(flag); };
        inline static void DrawArrays(RendererAPI::DrawMode mode, uint32_t count) { s_RendererAPI->DrawArrays(mode, count); };
        inline static void BindArrayTexture(uint32_t renderID, uint32_t slot) { s_RendererAPI->BindArrayTexture(renderID, slot); };
	private:
		static RendererAPI* s_RendererAPI;
	};
}
