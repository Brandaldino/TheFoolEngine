#pragma once

#include "Pass.h"

namespace TheFoolEngine
{
    // ====================== Direction & Spot Light Shadow ==================
    class ShadowPass : public Pass
    {
    public:
        virtual void Execute(RenderContext& context) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "ShadowPass"; // for debug
    private:
    };

    // ====================== Point Light Shadow =============================
    class PointShadowPass : public Pass
    {
    public:
        virtual void Execute(RenderContext& context) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "PointShadowPass"; // for debug
    };
}