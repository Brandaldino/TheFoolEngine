#pragma once

#include <glm/glm.hpp>

#include "TheFoolEngine/Core/Base.h"
#include "../../Renderer/Texture.h"

namespace TheFoolEngine
{

#define MAX_PBR_BONE_INFLUENCE 4

    struct PBRVertexData
    {
        // base
        glm::vec3 position;
        glm::vec2 texCoord;
        
        // TBN
        glm::vec3 tangent;
        glm::vec3 bitTangent;
        glm::vec3 normal;

        // bone
        int boneIDs[MAX_PBR_BONE_INFLUENCE];
        float weights[MAX_PBR_BONE_INFLUENCE];
    };

    struct PBRMeshData
    {
        std::vector<PBRVertexData> vertices;
        std::vector<uint32_t> indices;
        uint32_t MaterialIndex = 0;
        glm::mat4 NodeTransform = glm::mat4(1.0f);
    };

    struct PBRMaterialTextureSet
    {
        Ref<Texture2D> AlbedoMap;
        Ref<Texture2D> NormalMap;
        Ref<Texture2D> MetallicRoughnessMap;
        Ref<Texture2D> AOMap;

        glm::vec3 AlbedoFactor = glm::vec3(1.0f);
        float MetallicFactor = 0.0f;
        float RoughnessFactor = 1.0f;
        float AOStrength = 1.0f;
    };

    struct PBRMaterialData
    {
        std::vector<PBRMeshData>    Meshes;
        std::vector<PBRMaterialTextureSet> Textures;
    };
}