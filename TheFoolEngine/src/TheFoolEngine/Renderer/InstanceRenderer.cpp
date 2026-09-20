#include "tfpch.h"
#include "InstanceRenderer.h"

#include <glad/glad.h>

namespace TheFoolEngine
{

    InstanceRenderer::InstanceRenderer(uint32_t maxInstance)
        :m_MaxInstances(maxInstance)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferStorage(m_RendererID, m_MaxInstances * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    InstanceRenderer::~InstanceRenderer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void InstanceRenderer::Reset()
    {
        m_Offset = 0;
    }

    uint32_t InstanceRenderer::AddInstances(const std::vector<glm::mat4>& matrices)
    {
        if (matrices.empty())
            return m_Offset;

        if (m_Offset + (uint32_t)matrices.size() > m_MaxInstances)
        {
            TF_CORE_ERROR("InstanceRenderer overflow: {0}", m_Offset + matrices.size());
            return m_Offset;
        }

        uint32_t offset = m_Offset;
        glNamedBufferSubData(m_RendererID, offset * sizeof(glm::mat4), matrices.size() * sizeof(glm::mat4), matrices.data());
        m_Offset += (uint32_t)matrices.size();
        return offset;
    }

    void InstanceRenderer::BindRange(uint32_t offset, uint32_t count) const
    {
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_RendererID, offset * sizeof(glm::mat4), count * sizeof(glm::mat4));
    }

}