#include "tfpch.h"
#include "PBRMaterial.h"

namespace TheFoolEngine
{
    void PBRMaterialBuilder::Build(Ref<PBRMaterial> material)
    {
        uint32_t hash = GenerateHash(material->GetMaterialData());
        material->SetHash(hash);
    }
    uint32_t PBRMaterialBuilder::GenerateHash(const PBRMaterialTextureSet& data)
    {
        uint32_t hash = 0;

        auto combine = [&](uint32_t v) {
            hash ^= v + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            };

        auto hashTex = [&](const Ref<Texture2D>& tex) {
            combine(tex ? tex->GetRendererID() : 0);
            };

        auto hashFloat = [&](float f) {
            combine((uint32_t)std::hash<float>{}(f));
            };

        hashTex(data.AlbedoMap);
        hashTex(data.NormalMap);
        hashTex(data.MetallicRoughnessMap);
        hashTex(data.AOMap);

        hashFloat(data.AlbedoFactor.x);
        hashFloat(data.AlbedoFactor.y);
        hashFloat(data.AlbedoFactor.z);
        hashFloat(data.MetallicFactor);
        hashFloat(data.RoughnessFactor);
        hashFloat(data.AOStrength);

        return hash;
    }
}