#include "tfpch.h"
#include "OcclusionQuery.h"
#include <glad/glad.h>

namespace TheFoolEngine
{
    OcclusionQuery::OcclusionQuery()
    {
        glGenQueries(1, &m_RendererID);
    }

    OcclusionQuery::~OcclusionQuery()
    {
        glDeleteQueries(1, &m_RendererID);
    }

    void OcclusionQuery::Begin()
    {
        // CONSERVATIVE: consistent behavior across vendors (GL 4.6)
        glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_RendererID);
    }

    void OcclusionQuery::End()
    {
        glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
    }

    bool OcclusionQuery::Available()
    {
        GLint available = 0;
        glGetQueryObjectiv(m_RendererID, GL_QUERY_RESULT_AVAILABLE, &available);
        return available != 0;
    }

    uint32_t OcclusionQuery::GetResult()
    {
        GLuint result = 0;
        glGetQueryObjectuiv(m_RendererID, GL_QUERY_RESULT, &result);
        return result;
    }

}