#pragma once

#include "OcclusionQuery.h"
#include <glm/glm.hpp>
#include <unordered_set>

namespace TheFoolEngine
{
    enum class OcclusionState
    {
        Visible,
        RecentlyOccluded,
        Occluded,
    };

    struct OcclusionEntry
    {
        Ref<OcclusionQuery> Query = OcclusionQuery::Create();
        OcclusionState State = OcclusionState::Visible; // First frame render establishes the baseline
        uint32_t FramesSinceTest = 0;
        glm::vec3 BoundsCenter = glm::vec3(0.0f);
        glm::vec3 BoundsHalfExtents = glm::vec3(1.0f);
    };

    class OcclusionManager
    {
    public:
        void ReadBackResults(); // Frame start: read previous frame's query and update state
        void UpdateEntity(uint32_t id, const glm::vec3& center, const glm::vec3& halfExt);
        std::vector<uint32_t>& GetEntitiesToTest(); // Entities to submit for query this frame (grouped by state intervals)
        bool IsVisible(uint32_t id) const;  // Render set filtering
        OcclusionEntry& GetEntry(uint32_t id);
        void EndFrame();    // End of frame: FramesSinceTest++
        void RemoveEntity(uint32_t id); // Clean up entities that no longer exist
        void Prune(const std::unordered_set<uint32_t>& activeIDs);
    private:
        uint32_t GetTestInterval(OcclusionState state) const;
    private:
        std::unordered_map<uint32_t, OcclusionEntry> m_Entries; // id -> entry
        std::vector<uint32_t> m_TestCandidates; // every frame rebuild
    };
}