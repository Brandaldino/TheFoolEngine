#pragma once

#include "OcclusionQuery.h"
#include "TheFoolEngine/Core/UUID.h"

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
        void UpdateEntity(UUID id, const glm::vec3& center, const glm::vec3& halfExt);
        std::vector<UUID>& GetEntitiesToTest(); // Entities to submit for query this frame (grouped by state intervals)
        bool IsVisible(UUID id) const;  // Render set filtering
        OcclusionEntry& GetEntry(UUID id);
        void EndFrame();    // End of frame: FramesSinceTest++
        void RemoveEntity(UUID id); // Clean up entities that no longer exist
        void Prune(const std::unordered_set<UUID>& activeIDs);
    private:
        UUID GetTestInterval(OcclusionState state) const;
    private:
        std::unordered_map<UUID, OcclusionEntry> m_Entries; // id -> entry
        std::vector<UUID> m_TestCandidates; // every frame rebuild
    };
}