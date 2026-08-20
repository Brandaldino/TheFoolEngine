#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    // ====================== Direction & Spot Light Shadow ==================
    class ShadowPass : public Pass
    {
    public:
        virtual void Execute(RenderContext& context) override;
    private:

    };

    // ====================== Point Light Shadow =============================
    class PointShadowPass : public Pass
    {
    public:
        virtual void Execute(RenderContext& context) override;
    };
}