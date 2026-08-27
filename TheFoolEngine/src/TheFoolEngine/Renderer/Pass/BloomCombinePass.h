#pragma once

#include "Pass.h"
#include "../Shader.h"

namespace TheFoolEngine
{

    class BloomCombinePass : public Pass
    {
    public:
        BloomCombinePass(Ref<Shader> shader);

        void SetInputHDR(TextureHandle hdr);
        void SetInputBloom(TextureHandle bloom);
        virtual void SetOutput(TextureHandle output) override;

        virtual std::vector<TextureHandle>& GetInputs() override;
        virtual std::vector<TextureHandle>& GetOutputs() override;

        virtual void Execute(RenderContext& ctx) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "BloomCombinePass"; // for debug
    private:
        Ref<Shader> m_Shader;
        TextureHandle m_HDR, m_Bloom, m_Output;
        std::vector<TextureHandle> m_Inputs, m_Outputs;

        int m_HDRColor = 0, m_BloomBlur = 1;
        float m_Intensity = 1.0f;
    };

}