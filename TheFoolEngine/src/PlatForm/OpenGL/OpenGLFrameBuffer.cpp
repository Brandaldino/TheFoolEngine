#include "tfpch.h"
#include "OpenGLFrameBuffer.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace TheFoolEngine
{
	
	static const uint32_t s_MaxFrameBufferSize = 8192;

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
		: m_Specification(spec)
	{
		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
        m_DepthAttachment.reset();
        if (m_DepthArrayTextureID)
            glDeleteTextures(1, &m_DepthArrayTextureID);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
            m_ColorAttachment.reset();
            m_DepthAttachment.reset();
            if (m_DepthArrayTextureID)
                glDeleteTextures(1, &m_DepthArrayTextureID);
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        if (m_Specification.DepthOnly)
        {
            m_UseDepthArray = true;
            m_LayerCount = m_Specification.LayerCount;

            // GL_TEXTURE_2D_ARRAY
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_DepthArrayTextureID);
            glTextureStorage3D(m_DepthArrayTextureID, 1, GL_DEPTH_COMPONENT32F,
                m_Specification.Width, m_Specification.Height, m_Specification.LayerCount);
            glTextureParameteri(m_DepthArrayTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(m_DepthArrayTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(m_DepthArrayTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(m_DepthArrayTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            // border color set to 1.0 (when sampling at the boundary in the shader, it is considered "no shadow")
            float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(m_DepthArrayTextureID, GL_TEXTURE_BORDER_COLOR, border);

            // Hang on the 0th layer
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthArrayTextureID, 0, 0);
            m_CurrentLayer = 0;

            // without color
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
        else
        {
            m_ColorAttachment = Texture2D::Create(m_Specification.Width, m_Specification.Height, m_Specification.FrameBufferFormat);
            m_ColorAttachment->Bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment->GetRendererID(), 0);

            m_DepthAttachment = Texture2D::Create(m_Specification.Width, m_Specification.Height, AttachmentType::DepthStencil);
            uint32_t depthID = m_DepthAttachment->GetRendererID();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthID, 0);
        }

		TF_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "FrameBuffer is incomplite.")

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

    void OpenGLFrameBuffer::AttachLayer(uint32_t layer)
    {
        if (!m_UseDepthArray)
            return;
        m_CurrentLayer = layer;
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthArrayTextureID, 0, layer);
    }

	void OpenGLFrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFrameBuffer::UnBind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
		{
			TF_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

}
