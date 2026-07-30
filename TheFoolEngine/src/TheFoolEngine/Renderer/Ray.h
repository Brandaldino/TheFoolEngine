#pragma once

#include <glm/glm.hpp>

namespace TheFoolEngine
{
    namespace Ray
    {
        struct RayData
        {
            glm::vec3 Origin;
            glm::vec3 Direction;
        };

        RayData ScreenToRay(
            const glm::vec2& mousePos, 
            float viewportWidth, float viewportHeight,
            const glm::mat4& view, 
            const glm::mat4& proj);

        bool RayAABBIntersect(const RayData& ray, const glm::vec3& min, const glm::vec3& max, float& outDist);
    }

}

