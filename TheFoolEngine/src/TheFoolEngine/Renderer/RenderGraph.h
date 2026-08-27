#pragma once

#include "Pass/Pass.h"
#include "TextureHandle.h"

#include <vector>

namespace TheFoolEngine
{
    class RenderGraph
    {
    public:
        // === manage FBO resource =====================
        TextureHandle CreateRenderTarget(const TextureDesc& desc, const char* name);
        Ref<FrameBuffer> GetRenderTarget(const TextureHandle& handle) const;
        Ref<Texture2D> GetTexture(const TextureHandle& handle) const;
        void Resize(uint32_t width, uint32_t height);

        void AddPass(Scope<Pass> pass);
        void Execute(RenderContext& context);
    private:
        std::vector<uint32_t> TopologicalSort();
    private:
        std::vector<Scope<Pass>> m_Passes;
        std::vector<Ref<FrameBuffer>> m_FrameBuffers;
    };

}