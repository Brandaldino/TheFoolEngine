#include "tfpch.h"
#include "OpenGLOcclusionQuery.h"

#include <glad/glad.h>

namespace TheFoolEngine
{
    
    OpenGLOcclusionQuery::OpenGLOcclusionQuery()
    {
        glGenQueries(1, &m_RendererID);
    }

    OpenGLOcclusionQuery::~OpenGLOcclusionQuery()
    {
        glDeleteQueries(1, &m_RendererID);
    }

    void OpenGLOcclusionQuery::Begin()
    {
        // CONSERVATIVE: consistent behavior across vendors (GL 4.6)
        glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_RendererID);
    }

    void OpenGLOcclusionQuery::End()
    {
        glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
    }

    bool OpenGLOcclusionQuery::Available()
    {
        GLint available = 0;
        glGetQueryObjectiv(m_RendererID, GL_QUERY_RESULT_AVAILABLE, &available);
        return available != 0;
    }

    uint32_t OpenGLOcclusionQuery::GetResult()
    {
        GLuint result = 0;
        glGetQueryObjectuiv(m_RendererID, GL_QUERY_RESULT, &result);
        return result;
    }

    const uint32_t OpenGLOcclusionQuery::GetRendererID() const
    {
        return m_RendererID;
    }

}