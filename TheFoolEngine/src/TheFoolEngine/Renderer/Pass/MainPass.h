#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    class Shader;

    class MainPass : public Pass
    {
    public:
        MainPass(Ref<Shader> shader);

        virtual void SetOutput(TextureHandle output) override;

        virtual std::vector<TextureHandle>& GetInputs() override;
        virtual std::vector<TextureHandle>& GetOutputs() override;
        virtual void Execute(RenderContext& ctx) override;
        virtual std::string& GetPassName() { return m_PassName; };

        void SetInputShadow(TextureHandle handle);
    public:
        std::string m_PassName = "MainPass"; // for debug
    private:
        Ref<Shader> m_Shader;
        TextureHandle m_InputShadow, m_Output;
        std::vector<TextureHandle> m_Inputs, m_Outputs;
    };
}