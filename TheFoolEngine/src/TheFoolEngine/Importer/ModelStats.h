#pragma once

namespace TheFoolEngine
{
    struct MeshStats
    {
        uint32_t Vertices = 0;
        uint32_t Faces = 0;
        uint32_t Edges = 0; // Unique Edge Count (Deduplicated)
    };
}