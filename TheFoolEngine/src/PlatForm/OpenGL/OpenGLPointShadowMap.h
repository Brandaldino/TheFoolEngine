#pragma once

#include "../../TheFoolEngine/Renderer/PointShadowMap.h"

namespace TheFoolEngine
{
    class OpenGLPointShadowMap : public PointShadowMap
    {
    public:
        OpenGLPointShadowMap(uint32_t size);
        virtual ~OpenGLPointShadowMap();

        virtual void BindFace(uint32_t face) override;
        virtual void Bind(uint32_t slot) override;
        virtual void Unbind() override;
        virtual uint32_t GetRendererID() const override { return m_RendererID; };
    private:
        uint32_t m_RendererID;
        uint32_t m_FBO;
        uint32_t m_DepthRBO;
        uint32_t m_FaceSize;
    };
}