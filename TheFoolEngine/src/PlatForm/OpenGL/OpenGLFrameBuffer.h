#pragma once

#include "TheFoolEngine/Renderer/FrameBuffer.h"

namespace TheFoolEngine
{
	class OpenGLFrameBuffer :public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void UnBind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

        virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };

        virtual Ref<Texture2D> GetColorAttachment() override { return m_ColorAttachment; };
        virtual uint32_t GetRenderID() const override { return m_RendererID; };
	private:
		uint32_t m_RendererID = 0;
        uint32_t m_DepthAttachment;
        Ref<Texture2D> m_ColorAttachment;
		FrameBufferSpecification m_Specification;
	};
}
