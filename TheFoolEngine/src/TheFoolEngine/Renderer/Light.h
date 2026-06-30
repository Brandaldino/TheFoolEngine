#pragma once

#include <glm/glm.hpp>

namespace TheFoolEngine
{
    struct DirectionLight
    {
        glm::vec3 Direction;
        glm::vec3 Color;
        float Intensity;
    };

    struct PointLight
    {
        glm::vec3 Position;
        glm::vec3 Color;
        float Intensity;
        float Range;    // Decay distance: Beyond this distance, the contribution of light intensity is zero.
    };

    struct SpotLight
    {
        glm::vec3 Position;
        glm::vec3 Direction;
        glm::vec3 Color;
        float Intensity;
        float Range;
        float InnerAngle;
        float OuterAngle;
    };

    // === Only for definition, no actual functionality provided ==============
    struct RectLight
    {
        glm::vec3 Position;
        glm::vec3 Direction;
        glm::vec3 Up;
        glm::vec3 Color;
        float Intensity;
        float HalfWidth;
        float HalfHeight;
    };

    struct DiskLight
    {
        glm::vec3 Position;
        glm::vec3 Direction;
        glm::vec3 Up;
        glm::vec3 Color;
        float Intensity;
        float Radius;
    };

    // === Unified GPU Packed Architecture ======================================
    struct GPULight
    {
        glm::vec4 Position;
        glm::vec4 Direction;
        glm::vec4 Color;
        glm::vec4 Params;
    };

    enum class LightType : int
    {
        Directional = 0,
        Point,
        Spot,
        Rect,
        Disk
    };

}