#pragma once

#include "../Importer/PBRModel.h"
#include "../Importer/PBRMaterial/PBRMaterialManager.h"

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

    struct Light
    {
        glm::vec3 Position;
        glm::vec3 Color;
        float Intensity;
        int Type;   // type: 0 = directional, 1 = point
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
        static void SetLight(const Light& light);
        static void SetCamera(const CameraData& camera);

        static void Render();

        static void ResetStats();
        static PBRRenderState GetStats();
    };
}