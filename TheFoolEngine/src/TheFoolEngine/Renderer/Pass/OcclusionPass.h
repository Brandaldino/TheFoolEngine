#pragma once

#include "Pass.h"
#include "TheFoolEngine/Renderer/VertexArray.h"

namespace TheFoolEngine
{
    class Shader;

    class OcclusionPass : public Pass
    {
    public:
        OcclusionPass(Ref<Shader> shader);
        virtual std::vector<TextureHandle>& GetInputs() override;
        virtual std::vector<TextureHandle>& GetOutputs() override;
        virtual void Execute(RenderContext& context) override;
        virtual std::string& GetPassName() { return m_PassName; };

        void SetDepthHandle(TextureHandle& handle);
    public:
        std::string m_PassName = "OcclusionPass";
    private:
        Ref<Shader> m_Shader;
        Ref<VertexArray> m_CubeVAO;
        TextureHandle m_DepthHandle;
        std::vector<TextureHandle> m_Inputs, m_Outputs;
    };

}