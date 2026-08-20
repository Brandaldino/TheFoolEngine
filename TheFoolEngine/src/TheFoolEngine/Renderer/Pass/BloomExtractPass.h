#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    class BloomExtractPass : public Pass
    {
    public:
        void SetInput(Ref<FrameBuffer> src);
        void SetOutput(Ref<FrameBuffer> dst);
        void SetThreshold(float t);
        void Execute(RenderContext& ctx) override;
    };
}