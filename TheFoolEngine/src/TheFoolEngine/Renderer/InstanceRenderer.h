#pragma once

#include "../Core/Base.h"
#include <glm/glm.hpp>
#include <vector>

namespace TheFoolEngine
{
    class InstanceRenderer
    {
    public:
        InstanceRenderer(uint32_t maxInstance);
        ~InstanceRenderer();

        void Reset(); // Reset offset to zero at frame start
        uint32_t AddInstances(const std::vector<glm::mat4>& matrices);  // Fill and return the starting instance offset
        void BindRange(uint32_t offset, uint32_t count) const;

        const uint32_t GetRendererID() const { return m_RendererID; };
    private:
        uint32_t m_RendererID = 0;
        uint32_t m_MaxInstances = 0;
        uint32_t m_Offset = 0;
    };
}