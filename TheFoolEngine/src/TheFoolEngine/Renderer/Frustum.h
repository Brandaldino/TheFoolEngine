#pragma once
#include <glm/glm.hpp>

namespace TheFoolEngine
{
    class Frustum
    {
    public:
        struct Plane
        {
            glm::vec3 Normal;
            float D;
        };

        // Gribb-Hartmann: extract 6 planes from View*Projection (once per frame)
        inline void Extract(const glm::mat4& vp)
        {
            // Row-major convention (glm stores column-major; access as [col][row])
            m_Planes[0] = { { vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0] }, vp[3][3] + vp[3][0] };  // left
            m_Planes[1] = { { vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0] }, vp[3][3] - vp[3][0] };  // right
            m_Planes[2] = { { vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1] }, vp[3][3] + vp[3][1] };  // bottom
            m_Planes[3] = { { vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1] }, vp[3][3] - vp[3][1] };  // top
            m_Planes[4] = { { vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2] }, vp[3][3] + vp[3][2] };  // near
            m_Planes[5] = { { vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2] }, vp[3][3] - vp[3][2] };  // far
        }

        // AABB vs 6 planes (p-vertex test)
        inline bool Intersects(const glm::vec3& center, const glm::vec3& halfExtents) const
        {
            for (int i = 0; i < 6; ++i)
            {
                const auto& p = m_Planes[i];
                // p-vertex: the AABB corner most extreme along the plane normal
                glm::vec3 pv = center;
                pv.x += p.Normal.x > 0 ? halfExtents.x : -halfExtents.x;
                pv.y += p.Normal.y > 0 ? halfExtents.y : -halfExtents.y;
                pv.z += p.Normal.z > 0 ? halfExtents.z : -halfExtents.z;
                if (glm::dot(p.Normal, pv) + p.D < 0.0f)
                    return false;   // Entirely outside the plane → cull
            }
            return true;
        }
    private:
        Plane m_Planes[6];
    };
}