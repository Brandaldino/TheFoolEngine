#pragma once

#include "../../Core/Base.h"
#include "../FrameBuffer.h"
#include "../TextureHandle.h"
#include "../Light.h"
#include "../../Importer/PBRModel.h"

namespace TheFoolEngine
{
    struct PBRRenderProxy
    {
        std::string Name;
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

    struct RenderContext
    {
        CameraData Camera;      // camera
        glm::vec2 ViewportSize; // viewport
        std::vector<PBRRenderProxy> Renderables; // from register. TODO: Is it feasible to automatically manage whether the entities are rendered or not in each frame?
        std::vector<GPULight> Lights;   // lights
    };

    class Pass
    {
    public:
        virtual ~Pass() = default;

        virtual std::vector<TextureHandle>& GetInputs() { return m_Inputs; };
        virtual std::vector<TextureHandle>& GetOutputs() { return m_Outputs; };
        virtual void Execute(RenderContext& context) = 0;
    protected:
        std::vector<TextureHandle> m_Inputs;
        std::vector<TextureHandle> m_Outputs;
    };
}