#pragma once

#include "Pass/Pass.h"
#include "TextureHandle.h"

#include <vector>

namespace TheFoolEngine
{
    class PointShadowMap;

    class RenderGraph
    {
    public:
        // === manage FBO resource =====================
        TextureHandle CreateRenderTarget(const TextureDesc& desc, const char* name);
        Ref<FrameBuffer> GetFrameBuffer(const TextureHandle& handle) const;    // Color | DepthArray
        Ref<PointShadowMap> GetPointShadowMap(const TextureHandle& handle) const;   // CubeMapArray
        Ref<Texture2D> GetTexture(const TextureHandle& handle) const;
        void Resize(uint32_t width, uint32_t height);

        void AddPass(Scope<Pass> pass);
        void Execute(RenderContext& context);
    public:
        // === for debug ========================
        uint32_t GetResourceSize() const { return (uint32_t)m_Resources.size(); };
    private:
        std::vector<uint32_t> TopologicalSort();
    private:
        struct RenderTargetResource
        {
            TextureDesc Desc;
            Ref<FrameBuffer> FrameBuffer;   // Color / DepthArray
            Ref<PointShadowMap> PointShadowMap; // CubeMapArray
            uint32_t PoolIndex = 0;
        };
        std::vector<RenderTargetResource> m_Resources;

        std::vector<Scope<Pass>> m_Passes;
        // std::vector<Ref<FrameBuffer>> m_FrameBuffers;
    };

}