#pragma once

#include "TheFoolEngine/Renderer/RenderUtil.h"

namespace TheFoolEngine 
{

    class OpenGLRenderUtil : public RenderUtil
    {
    public:
        virtual void SaveViewport(int prev[4]) override;
        virtual void RestoreViewport(const int prev[4]) override;
        virtual Ref<VertexArray> CreateCubeVAO() override;
        virtual Ref<VertexArray> CreateFullscreenQuadVAO() override;
        virtual void RenderToCubemapFaces(
            Ref<CubeMap> dst, uint32_t faceSize,
            Ref<Shader> shader,
            const glm::mat4& projection,
            std::function<void(int face, const glm::mat4& view)> bindUniforms,
            int mipLevel = 0) override;
        virtual void RenderToTexture2D(
            Ref<Texture2D> texture, uint32_t size,
            Ref<Shader> shader,
            std::function<void()> bindUniforms) override;
    };

}
