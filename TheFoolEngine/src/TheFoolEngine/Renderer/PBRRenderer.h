#pragma once

#include "../Importer/PBRModel.h"
#include "../Importer/PBRMaterial/PBRMaterialManager.h"

#include "Light.h"
#include "CubeMap.h"

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

        static void Render(const RenderContext& context);

        static void ResetStats();
        static PBRRenderState GetStats();

        static void DefaultTextureFill(Ref<PBRModel> model);

        static void SetSkybox(const Ref<CubeMap> skybox);
        static void SetEnvironmentMap(const Ref<CubeMap> irradiance, const Ref<CubeMap> prefilter, const Ref<Texture2D> brdfLUT);

        // ============= MainPass ==================================
        static Ref<Shader> GetPBRShader();
        static const PBRMaterialTextureSet& GetDefaultTexture();
        static const EnvironmentData& GetEnvironment();
    };
}