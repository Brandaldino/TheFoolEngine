#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    class Shader;

    // ====================== Direction & Spot Light Shadow ==================
    class ShadowPass : public Pass
    {
    public:
        virtual void SetOutput(TextureHandle& handle);
        virtual std::vector<TextureHandle>& GetOutputs();

        virtual void Execute(RenderContext& context) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "ShadowPass"; // for debug
    private:
        Ref<Shader> m_Shader;
        TextureHandle m_Output;
        std::vector<TextureHandle> m_Outputs;
    };

    // ====================== Point Light Shadow =============================
    class PointShadowPass : public Pass
    {
    public:
        virtual void Execute(RenderContext& context) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "PointShadowPass"; // for debug
    private:
        Ref<Shader> m_Shader;
    };
}