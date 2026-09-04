#pragma once

#include "Pass.h"
#include "../Shader.h"

namespace TheFoolEngine
{

    class BloomBlurPass : public Pass
    {
    public:
        BloomBlurPass(Ref<Shader> shader);

        void SetDirection(const glm::vec2& dir);

        virtual void SetInput(TextureHandle& input) override;
        virtual void SetOutput(TextureHandle& output) override;

        virtual std::vector<TextureHandle>& GetInputs() override;
        virtual std::vector<TextureHandle>& GetOutputs() override;
        virtual void Execute(RenderContext& ctx) override;

        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "BloomBlurPass"; // for debug
    private:
        Ref<Shader> m_Shader;
        TextureHandle m_Input, m_Output;
        std::vector<TextureHandle> m_Inputs, m_Outputs;

        glm::vec2 m_Direction;
    };
}