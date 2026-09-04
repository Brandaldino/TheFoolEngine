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
    }
}