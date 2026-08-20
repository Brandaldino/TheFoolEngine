#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    class Shader;

    class MainPass : public Pass
    {
    public:
        virtual std::vector<TextureHandle>& GetOutputs() override;
        virtual void Execute(RenderContext& ctx) override;

        void SetTarget(Ref<FrameBuffer> target, Ref<Shader> shader);
    private:
        Ref<Shader> m_Shader;
        Ref<FrameBuffer> m_OutputFBO;
    };
}