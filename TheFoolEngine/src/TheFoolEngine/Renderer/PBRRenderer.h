#pragma once

#include "../Importer/PBRModel.h"
#include "../Importer/PBRMaterial/PBRMaterialManager.h"

#include "Light.h"
#include "CubeMap.h"

#include "PointShadowMap.h"

#include "FrameBuffer.h"
#include "Pass/Pass.h"

namespace TheFoolEngine
{
    class Shader;
    class PerspectiveCamera;

    constexpr uint32_t MAX_SHADOW_LIGHTS = 8;
    constexpr uint32_t NR_LIGHTS = 10;
    constexpr uint32_t MAX_TEXTURE_SLOTS = 32;
    constexpr uint32_t SHADOWMAP_SIZE = 1024;

    struct LightGPUBlock
    {
        GPULight Lights[NR_LIGHTS];
        int32_t LightCount;
    };

    struct EnvironmentData
    {
        Ref<CubeMap> Skybox;
        Ref<Shader> SkyboxShader;
        Ref<VertexArray> SkyboxCubeVAO;
        Ref<CubeMap> IrradianceMap;
        Ref<CubeMap> PrefilterMap;
        Ref<Texture2D> BRDFLUT;
    };

    struct ShadowData
    {
        Ref<FrameBuffer> ShadowFBO;
        Ref<Shader> DepthOnlyShader;
        std::vector<glm::mat4> LightViewProjections;  // lightProj * lightView
    };

    struct PointLightShadowData
    {
        glm::vec3 LightPosition;
        glm::mat4 ShadowViews[6];
        glm::mat4 ShadowProj;
        float FarPlane;
    };

    struct PointShadowData
    {
        Ref<Shader> DepthShader;
        Ref<PointShadowMap> DepthMap;
        PointLightShadowData Lights[MAX_SHADOW_LIGHTS];
        uint32_t Count = 0;
    };

    class PBRRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void ResetRendererState();

        static void AddLight(RenderContext& context, const DirectionLight& light);
        static void AddLight(RenderContext& context, const DirectionLight& light, int shadowIndex);
        static void AddLight(RenderContext& context, const PointLight& light);
        static void AddLight(RenderContext& context, const PointLight& light, int shadowIndex);
        static void AddLight(RenderContext& context, const SpotLight& light);
        static void AddLight(RenderContext& context, const SpotLight& light, int shadowIndex);

        static void Render(const RenderContext& context);

        static void ResetStats();
        static PBRRenderState GetStats();

        static std::int32_t GetLightsCount(const RenderContext& context);

        static void DefaultTextureFill(Ref<PBRModel> model);

        static void SetSkybox(const Ref<CubeMap> skybox);
        static void SetEnvironmentMap(const Ref<CubeMap> irradiance, const Ref<CubeMap> prefilter, const Ref<Texture2D> brdfLUT);

        static int SetShadowLight(const glm::vec3& lightDir, float orthoSize = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f);
        static int SetShadowLight(RenderContext& context, const glm::vec3& lightDir, float orthoSize = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f);
        static int SetSpotShadowLight(const glm::vec3& position, const glm::vec3& direction, float fovDeg, float nearPlane = 0.1f, float farPlane = 100.0f);
        static int SetPointShadowLight(const glm::vec3& position, float nearPlane = 0.1f, float farPlane = 100.0f);

        static void RenderShadowPass(const RenderContext& context);
        static void RenderPointShadowPass(const RenderContext& context);

        // ============= MainPass ==================================
        static Ref<Shader> GetPBRShader();
        static const PBRMaterialTextureSet& GetDefaultTexture();
        static uint32_t GetLightUBO();
        static const EnvironmentData& GetEnvironment();
        // ============= ShadowPass ================================
        static Ref<FrameBuffer>  GetShadowFBO();
        static Ref<Shader> GetDepthOnlyShader();
        static const std::vector<glm::mat4>& GetShadowViewProjections();
        // ============= PointShadowPass ===========================
        static Ref<Shader> GetPointShadowDepthShader();
        static Ref<PointShadowMap> GetPointShadowMap();
        static const PointShadowData& GetPointShadowData();
    };
}