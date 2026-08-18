#include "tfpch.h"
#include "OpenGLPointShadowMap.h"

#include <glad/glad.h>

namespace TheFoolEngine
{

    OpenGLPointShadowMap::OpenGLPointShadowMap(uint32_t size)
        :m_FaceSize(size)
    {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, GL_R32F, size, size);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // FBO
        glCreateFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Depth renderbuffer: used only for depth testing to preserve the closest face; actual depth is stored in an R32F color attachment
        glCreateRenderbuffers(1, &m_DepthRBO);
        glNamedRenderbufferStorage(m_DepthRBO, GL_DEPTH_COMPONENT24, size, size);
        glNamedFramebufferRenderbuffer(m_FBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRBO);
    }

    OpenGLPointShadowMap::~OpenGLPointShadowMap()
    {
        glDeleteTextures(1, &m_RendererID);
        glDeleteFramebuffers(1, &m_FBO);
        glDeleteRenderbuffers(1, &m_DepthRBO);
    }

    void OpenGLPointShadowMap::BindFace(uint32_t face)
    {
        TF_PROFILE_FUNCTION();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_RendererID, 0);
    }

    void OpenGLPointShadowMap::Bind(uint32_t slot)
    {
        TF_PROFILE_FUNCTION();

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        // glBindTextureUnit(slot, m_RendererID);
        glViewport(0, 0, m_FaceSize, m_FaceSize);
    }

    void OpenGLPointShadowMap::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}