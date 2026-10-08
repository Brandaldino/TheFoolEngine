#pragma once

#include "../Core/Base.h"

#include <glm/gtc/matrix_transform.hpp>
#include "ShadowTypes.h"

namespace TheFoolEngine
{
    namespace ShadowMath
    {
        inline glm::mat4 ComputeDirLightVP(const glm::vec3 & dir, float ortho = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f)
        {
            glm::vec3 sceneCenter = glm::vec3(0.0f);
            glm::vec3 lightPos = sceneCenter - dir * 50.0f;
            glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 lightProj = glm::ortho(-ortho, ortho, -ortho, ortho, nearPlane, farPlane);
            return lightProj * lightView;
        }

        inline glm::mat4 ComputeSpotLightVP(const glm::vec3 & pos, const glm::vec3 & dir, float fov, float nearPlane = 0.1f, float farPlane = 100.0f)
        {
            glm::vec3 up = (glm::abs(glm::dot(dir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
                ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 lightView = glm::lookAt(pos, pos + dir, up);
            glm::mat4 lightProj = glm::perspective(glm::radians(fov), 1.0f, nearPlane, farPlane);

            return lightProj * lightView;
        }

        inline PointLightShadowData ComputePointLightShadowData(const glm::vec3 & pos, float nearPlane = 0.1f, float farPlane = 100.0f)
        {
            PointLightShadowData data;
            data.LightPosition = pos;
            data.FarPlane = farPlane;
            data.ShadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

            data.ShadowViews[0] = glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            data.ShadowViews[1] = glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            data.ShadowViews[2] = glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            data.ShadowViews[3] = glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            data.ShadowViews[4] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            data.ShadowViews[5] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

            return data;
        }

        // Geometric + linear blend (UE5 split scheme, controlled by lambda)
        inline float CascadeSplit(float nearClip, float farClip, float lambda, uint32_t cascade, uint32_t cascadeCount)
        {
            float p = (float)(cascade + 1) / (float)cascadeCount;
            float logSplit = nearClip * pow(farClip / nearClip, p);     // Geometric
            float linearSplit = nearClip + (farClip - nearClip) * p;    // Linear
            return glm::mix(logSplit, linearSplit, lambda);             // blend
        }

        // Cascade VP (slice → light-space AABB → ortho)
        inline glm::mat4 ComputeDirLightCascadeVP(
            const glm::vec3& lightDir,
            const glm::vec3& camPos,
            const glm::vec3& camForward,
            const glm::vec3& camUp,
            float verticalFovRad, float aspect, // ← Use vertical FOV (consistent with projection; previous pitfall)
            float cascadeNear, float cascadeFar,
            const glm::vec3& sceneBoundsMin, const glm::vec3& sceneBoundsMax)   // Scene model AABB
        {
            // 1. 8 corners of the camera frustum slice (using vertical FOV)
            glm::vec3 right = glm::normalize(glm::cross(camForward, camUp));
            glm::vec3 up = glm::normalize(glm::cross(right, camForward));
            float tanFov = tan(verticalFovRad * 0.5f);
            glm::vec3 fwdNear = camPos + camForward * cascadeNear;
            glm::vec3 fwdFar = camPos + camForward * cascadeFar;
            glm::vec3 hNear = right * (tanFov * aspect * cascadeNear);
            glm::vec3 vNear = up * (tanFov * cascadeNear);
            glm::vec3 hFar = right * (tanFov * aspect * cascadeFar);
            glm::vec3 vFar = up * (tanFov * cascadeFar);
            glm::vec3 corners[8] = {
                fwdNear - hNear - vNear, fwdNear + hNear - vNear,
                fwdNear - hNear + vNear, fwdNear + hNear + vNear,
                fwdFar - hFar - vFar,  fwdFar + hFar - vFar,
                fwdFar - hFar + vFar,  fwdFar + hFar + vFar,
            };

            // 2. lightview: base light position on scene bounds so the whole scene lies in front (all light-space z negative)
            glm::vec3 sceneCenter = (sceneBoundsMin + sceneBoundsMax) * 0.5f;
            float sceneRadius = glm::length(sceneBoundsMax - sceneBoundsMin) * 0.5f;
            glm::vec3 lightPos = sceneCenter - lightDir * (sceneRadius * 2.0f + 100.0f);
            glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 1.0f, 0.0f));

            // 3. Frustum slice corners → light-space AABB
            glm::vec3 minB(1e30f), maxB(-1e30f);
            for (auto& corner : corners)
            {
                glm::vec4 lc = lightView * glm::vec4(corner, 1.0f);
                minB = glm::min(minB, glm::vec3(lc));
                maxB = glm::max(maxB, glm::vec3(lc));
            }

            // 4. Padding (once outside the loop — previous pitfall)
            // Model AABB → light space, merge (extend Z depth range only)
            glm::vec3 mCorners[8] = {
                sceneBoundsMin,
                {sceneBoundsMax.x, sceneBoundsMin.y, sceneBoundsMin.z},
                {sceneBoundsMin.x, sceneBoundsMax.y, sceneBoundsMin.z},
                {sceneBoundsMax.x, sceneBoundsMax.y, sceneBoundsMin.z},
                {sceneBoundsMin.x, sceneBoundsMin.y, sceneBoundsMax.z},
                {sceneBoundsMax.x, sceneBoundsMin.y, sceneBoundsMax.z},
                {sceneBoundsMin.x, sceneBoundsMax.y, sceneBoundsMax.z},
                sceneBoundsMax,
            };
            for (auto& c : mCorners)
            {
                glm::vec4 lc = lightView * glm::vec4(c, 1.0f);
                minB.z = glm::min(minB.z, lc.z);
                maxB.z = glm::max(maxB.z, lc.z);
            }
            float padding = 20.0f;
            minB -= padding;
            maxB += padding;

            // 5. Ortho: light-space z is all negative (OpenGL -Z), flip to positive depth
            // Nearest = largest z, furthest = smallest z
            float nearDepth = -maxB.z;   // nearest → positive depth
            float farDepth = -minB.z;    // furthest → positive depth
            glm::mat4 lightProj = glm::ortho(minB.x, maxB.x, minB.y, maxB.y, nearDepth, farDepth);
            return lightProj * lightView;
        }
    }
}