#pragma once

#include "VertexArray.h"
#include "CubeMap.h"
#include "Shader.h"
#include "Texture.h"

namespace TheFoolEngine
{
    class RenderUtil
    {
    public:
        virtual ~RenderUtil() = default;

        virtual void SaveViewport(int prev[4]) = 0;
        virtual void RestoreViewport(const int prev[4]) = 0;
        virtual Ref<VertexArray> CreateCubeVAO() = 0;
        virtual Ref<VertexArray> CreateFullscreenQuadVAO() = 0;
        virtual void RenderToCubemapFaces(
            Ref<CubeMap> dst, uint32_t faceSize,
            Ref<Shader> shader, const glm::mat4& projection,
            std::function<void(int face, const glm::mat4& view)> bindUniforms,
            int miplevel = 0) = 0;
        virtual void RenderToTexture2D(
            Ref<Texture2D> texture, uint32_t size,
            Ref<Shader> shader,
            std::function<void()> bindUniforms) = 0;

        static RenderUtil* Get();
    private:
        static RenderUtil* s_Instance;
    };
}