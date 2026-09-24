#pragma once

#include "TheFoolEngine/Renderer/OcclusionQuery.h"

namespace TheFoolEngine
{
    class OpenGLOcclusionQuery : public OcclusionQuery
    {
    public:
        OpenGLOcclusionQuery();
        virtual ~OpenGLOcclusionQuery();

        virtual void Begin() override;
        virtual void End() override;
        virtual bool Available() override;
        virtual uint32_t GetResult() override;
        virtual const uint32_t GetRendererID() const override;
    private:
        uint32_t m_RendererID;
    };
}