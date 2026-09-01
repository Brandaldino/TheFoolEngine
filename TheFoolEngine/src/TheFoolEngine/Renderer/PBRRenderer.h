#pragma once

#include "../Importer/PBRModel.h"
#include "../Importer/PBRMaterial/PBRMaterialManager.h"

#include "Light.h"
#include "CubeMap.h"

#include "PointShadowMap.h"

#include "FrameBuffer.h"
#include "Pass/Pass.h"

#include "ShadowTypes.h"

namespace TheFoolEngine
{
    class Shader;
    class PerspectiveCamera;

    constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

    struct EnvironmentData
    {
        Ref<CubeMap> Skybox;
        Ref<Shader> SkyboxShader;
        Ref<VertexArray> SkyboxCubeVAO;
        Ref<CubeMap> IrradianceMap;
        Ref<CubeMap> PrefilterMap;
        Ref<Texture2D> BRDFLUT;
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