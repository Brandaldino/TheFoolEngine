#include "tfpch.h"
#include "ModelAnalyzer.h"

#include <unordered_set>

namespace TheFoolEngine
{
    MeshStats ModelAnalyzer::ComputeMeshStats(const PBRMaterialData& data)
    {
        MeshStats stats;
        for (auto& mesh : data.Meshes)
        {
            stats.Vertices += (uint32_t)mesh.vertices.size();
            stats.Faces += (uint32_t)(mesh.indices.size() / 3);

            std::unordered_set<uint64_t> edgeSet;
            for (std::size_t f = 0; f < mesh.indices.size(); f += 3)
            {
                uint32_t tri[3] = { mesh.indices[f], mesh.indices[f + 1], mesh.indices[f + 2] };
                for (int e = 0; e < 3; ++e)
                {
                    uint32_t a = tri[e], b = tri[(e + 1) % 3];
                    if (a > b)
                        std::swap(a, b);
                    edgeSet.insert(((uint64_t)a << 32) | b);
                }
            }
            stats.Edges += (uint32_t)edgeSet.size();
        }

        return stats;
    }
}