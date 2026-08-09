#pragma once

#include "TheFoolEngine/Renderer/FrameBuffer.h"

namespace TheFoolEngine
{
	class OpenGLFrameBuffer :public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		virtual void Bind() override;
		virtual void UnBind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

        virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };

        virtual Ref<Texture2D> GetColorAttachment() override { return m_ColorAttachment; };
        virtual Ref<Texture2D> GetDepthAttachment() override { return m_DepthAttachment; };
        virtual uint32_t GetRenderID() const override { return m_RendererID; };

        virtual uint32_t GetDepthArrayTextureID() override { return m_DepthArrayTextureID; };
        virtual void AttachLayer(uint32_t layer) override; // Switch the current mounted layer

        void Invalidate();
	private:
		uint32_t m_RendererID = 0;
        Ref<Texture2D> m_ColorAttachment;
        Ref<Texture2D> m_DepthAttachment;
		FrameBufferSpecification m_Specification;

        uint32_t m_DepthArrayTextureID = 0;   // GL_TEXTURE_2D_ARRAY ID
        uint32_t m_CurrentLayer = 0;
        bool m_UseDepthArray = false;
        uint32_t m_LayerCount = 0;
	};
}
