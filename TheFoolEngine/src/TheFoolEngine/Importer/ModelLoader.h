#pragma once

#include <filesystem>

#include "MaterialData/PBRMaterialData.h"

namespace TheFoolEngine
{

    class AssimpImporter
    {
    public:
        AssimpImporter() = default;
        ~AssimpImporter() = default;

        static PBRMaterialData Import(const std::filesystem::path& path);
    };
}