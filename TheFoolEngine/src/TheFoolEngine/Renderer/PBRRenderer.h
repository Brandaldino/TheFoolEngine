#pragma once

#include "../Importer/PBRModel.h"
#include "../Importer/PBRMaterial/PBRMaterialManager.h"

#include "Light.h"
#include "CubeMap.h"

class Shader;
class PerspectiveCamera;

namespace TheFoolEngine
{
    struct PBRRenderProxy
    {
        Ref<PBRModel> Model;
        glm::mat4 Transform = glm::mat4(1.0f);
        bool Visible = true;
    };

    struct CameraData
    {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 Position;
    };

    struct PBRRenderState
    {
        uint32_t DrawCalls = 0;
        uint32_t MeshCount = 0;
    };

    class PBRRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void ResetRendererState();
        static void Register(const PBRRenderProxy& proxy);
        static void SetCamera(const CameraData& camera);
        static void AddLight(const DirectionLight& light);
        static void AddLight(const PointLight& light);
        static void AddLight(const SpotLight& light);

        static void Render();

        static void ResetStats();
        static PBRRenderState GetStats();

        static std::int32_t GetLightsCount();

        static void DefaultTextureFill(Ref<PBRModel> model);

        static void SetSkybox(const Ref<CubeMap> skybox);
    };
}