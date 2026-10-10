#pragma once

#include "ModelStats.h"
#include "MaterialData/PBRMaterialData.h"

namespace TheFoolEngine
{
    class ModelAnalyzer
    {
    public:
        static MeshStats ComputeMeshStats(const PBRMaterialData& data);
    };
}