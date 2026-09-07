#include "tfpch.h"
#include "LightPacker.h"

#include <glad/glad.h>

namespace TheFoolEngine
{

    namespace LightPacker
    {

        GPULight PackDirection(const DirectionLight& light, int shadowIndex)
        {
            GPULight gpu;
            gpu.Position = glm::vec4(light.Direction * 1e6f, 0.0f);
            gpu.Direction = glm::vec4(light.Direction, 0.0f);
            gpu.Color = glm::vec4(light.Color, light.Intensity);
            gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Directional);
            gpu.ShadowIndex = shadowIndex;

            return gpu;
        }

        GPULight PackSpot(const SpotLight& light, int shadowIndex)
        {
            float innerCos = glm::cos(light.InnerAngle);
            float outerCos = glm::cos(light.OuterAngle);

            GPULight gpu;
            gpu.Position = glm::vec4(light.Position, light.Range);
            gpu.Direction = glm::vec4(light.Direction, 0.0f);
            gpu.Color = glm::vec4(light.Color, light.Intensity);
            gpu.Params = glm::vec4(light.Range, innerCos, outerCos, (float)LightType::Spot);
            gpu.ShadowIndex = shadowIndex;

            return gpu;
        }

        GPULight PackPoint(const PointLight& light, int shadowIndex)
        {
            GPULight gpu;
            gpu.Position = glm::vec4(light.Position, light.Range);
            gpu.Direction = glm::vec4(0.0f);
            gpu.Color = glm::vec4(light.Color, light.Intensity);
            gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Point);
            gpu.ShadowIndex = shadowIndex;

            return gpu;
        }

        void SetGPULightFBO(uint32_t ubo, std::vector<GPULight> lights)
        {
            LightGPUBlock lightblock = {};
            lightblock.LightCount = (int32_t)lights.size();
            for (int32_t i = 0;i < lightblock.LightCount && i < NR_LIGHTS; ++i)
            {
                lightblock.Lights[i].Position = lights[i].Position;
                lightblock.Lights[i].Direction = lights[i].Direction;
                lightblock.Lights[i].Color = lights[i].Color;
                lightblock.Lights[i].Params = lights[i].Params;
                lightblock.Lights[i].ShadowIndex = lights[i].ShadowIndex;
            }
            glNamedBufferSubData(ubo, 0, sizeof(LightGPUBlock), &lightblock);
        }

    }

}