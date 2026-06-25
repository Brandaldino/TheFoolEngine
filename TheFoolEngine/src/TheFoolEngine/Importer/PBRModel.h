#pragma once

#include "MaterialData/PBRMaterialData.h"

#include <filesystem>

#include "../Renderer/VertexArray.h"

namespace TheFoolEngine
{
    class PBRModel
    {
    public:
        PBRModel();
        ~PBRModel();

        void Import(std::filesystem::path& path);
        void UpLoad();
        void Release();

        const std::filesystem::path& GetPath() const { return m_FilePath; };
        const PBRMaterialData& GetModelData() const { return m_ModelData; };
        const std::vector<Ref<VertexArray>>& GetVertexArray() const { return m_VertexArray; };
    private:
        std::filesystem::path m_FilePath;
        PBRMaterialData m_ModelData;
        std::vector<Ref<VertexArray>> m_VertexArray;
    };
}