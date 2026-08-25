#pragma once

#include "Pass/Pass.h"
#include "TextureHandle.h"
#include "ResourcePool.h"

#include <vector>

namespace TheFoolEngine
{
    class RenderGraph
    {
    public:
        TextureHandle CreateTexture(const TextureDesc& desc, const char* name);
        TextureHandle RegisterTexture(const TextureDesc& desc, Ref<Texture2D> texture, const char* name);

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
        std::vector<TextureHandle> m_Resources; // for debug
        ResourcePool m_Pool;

        std::vector<Ref<FrameBuffer>> m_FrameBuffers;
        std::vector<TextureDesc> m_Descs;
    };

}