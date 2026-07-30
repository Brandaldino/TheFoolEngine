#include "tfpch.h"
#include "Ray.h"

namespace TheFoolEngine
{

    namespace Ray
    {
        RayData ScreenToRay(const glm::vec2& mousePos, float viewportWidth, float viewportHeight, const glm::mat4& view, const glm::mat4& proj)
        {
            float ndcX = (2.0f * mousePos.x) / viewportWidth - 1.0f;
            float ndcY = 1.0f - (2.0f * mousePos.y) / viewportHeight;

            glm::vec4 clipPos(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 viewPos = glm::inverse(proj) * clipPos;
            viewPos /= viewPos.w;

            glm::vec4 worldPos = glm::inverse(view) * viewPos;

            RayData res;
            glm::mat4 invView = glm::inverse(view);
            res.Origin = glm::vec3(invView[3]);
            res.Direction = glm::normalize(glm::vec3(worldPos) - res.Origin);

            return res;
        }

        // slab algorithm
        bool RayAABBIntersect(const RayData& ray, const glm::vec3& min, const glm::vec3& max, float& outDist)
        {
            float tEnter = -1e30f;
            float tExit = 1e30f;

            for (int i = 0; i < 3; ++i)
            {
                float origin = ray.Origin[i];
                float dir = ray.Direction[i];

                if (fabs(dir) < 1e-10f)
                {
                    if (origin < min[i] || origin > max[i])
                        return false;
                    // Parallel and within the slab → Unconstrained, skip
                    continue;
                }

                float t1 = (min[i] - origin) / dir;
                float t2 = (max[i] - origin) / dir;

                tEnter = glm::max(tEnter, glm::min(t1, t2));
                tExit = glm::min(tExit, glm::max(t1, t2));
            }

            if (tEnter < tExit && tExit > 0.0f)
            {
                outDist = tEnter > 0.0f ? tEnter : tExit; // The situation where the rays originate from within
                return true;
            }
            return false;
        }

    }

}
