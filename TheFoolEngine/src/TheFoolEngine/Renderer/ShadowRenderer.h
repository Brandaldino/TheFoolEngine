#pragma once

#include "FrameBuffer.h"
#include "Shader.h"
#include "PointShadowMap.h"

#include "Pass/Pass.h"
#include "ShadowTypes.h"
#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{
    class RenderGraph;

    class ShadowRenderer
    {
    public:
        void Init(RenderGraph* graph);
        void Reset(); // every frame reset

        // data set
        int SetDirectionalLight(const glm::vec3& dir, float ortho = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f);
        int SetSpotLight(const glm::vec3& pos, const glm::vec3& dir, float fov, float nearPlane = 0.1f, float farPlane = 100.0f);
        int SetPointLight(const glm::vec3& pos, float nearPlane = 0.1f, float farPlane = 100.0f);
        void SetGPULightFBO(RenderContext& context);

        // data upload
        void AddDirectionalLight(RenderContext& context, const DirectionLight& light, int shadowIndex = -1);
        void AddSpotLight(RenderContext& context, const SpotLight& light, int shadowIndex = -1);
        void AddPointLight(RenderContext& context, const PointLight& light, int shadowIndex = -1);

        // get resource
        Ref<Shader> GetDepthShader();
        Ref<Shader> GetPointDepthShader();
        const std::vector<glm::mat4>& GetViewProjections();
        const PointShadowData& GetPointShadowData();
        uint32_t GetLightUBO();

        TextureHandle GetShadowFBOHandle();
        TextureHandle GetPointShadowHandle();
    private:
        ShadowData m_ShadowData;
        PointShadowData m_PointShadowData;
        uint32_t m_GPULightUBO;

        TextureHandle m_ShadowFBOHandle;
        TextureHandle m_PointShadowHandle;
    };

}