#include "tfpch.h"
#include "OcclusionManager.h"

namespace TheFoolEngine
{

    void OcclusionManager::ReadBackResults()
    {
        for (auto& [id, entry] : m_Entries)
        {
            // Result not ready (GPU is slow) → keep current state, no update
            if (!entry.Query->Available())
                continue;
            uint32_t samples = entry.Query->GetResult();
            if (samples > 0)
                entry.State = OcclusionState::Visible;  // Any pixels passed → visible (force Visible regardless of previous state)
            else
            {
                // Occluded → gradual downgrade (UE5 approach)
                if (entry.State == OcclusionState::Visible)
                    entry.State = OcclusionState::RecentlyOccluded; // Progressive
                else if (entry.State == OcclusionState::RecentlyOccluded)
                    entry.State = OcclusionState::Occluded;
            }

            entry.FramesSinceTest = 0; // reset time
        }
    }

    void OcclusionManager::UpdateEntity(UUID id, const glm::vec3& center, const glm::vec3& halfExt)
    {
        auto& entry = m_Entries[id];
        entry.BoundsCenter = center;
        entry.BoundsHalfExtents = halfExt;
    }

    std::vector<UUID>& OcclusionManager::GetEntitiesToTest()
    {
        m_TestCandidates.clear();
        for (auto& [id, entry] : m_Entries)
        {
            UUID interval = GetTestInterval(entry.State);
            if (entry.FramesSinceTest >= interval)
                m_TestCandidates.push_back(id);
        }
        return m_TestCandidates;
    }

    bool OcclusionManager::IsVisible(UUID id) const
    {
        auto it = m_Entries.find(id);
        if (it == m_Entries.end())
            return true;    // Unregistered (first frame / new entity) → conservative rendering
        return it->second.State == OcclusionState::Visible;
    }

    OcclusionEntry& OcclusionManager::GetEntry(UUID id)
    {
        return m_Entries[id];
    }

    void OcclusionManager::EndFrame()
    {
        for (auto& [id, entry] : m_Entries)
            entry.FramesSinceTest++;    // Increment frame counter++
    }

    void OcclusionManager::RemoveEntity(UUID id)
    {
        m_Entries.erase(id);
    }

    void OcclusionManager::Prune(const std::unordered_set<UUID>& activeIDs)
    {
        for (auto it = m_Entries.begin(); it != m_Entries.end();)
        {
            if (activeIDs.find(it->first) == activeIDs.end())
                it = m_Entries.erase(it);
            else
                ++it;
        }
    }

    UUID OcclusionManager::GetTestInterval(OcclusionState state) const
    {
        switch (state)
        {
            case OcclusionState::Visible: return 1;
            case OcclusionState::RecentlyOccluded: return 8;
            case OcclusionState::Occluded: return 30;
        }
        return 1;
    }

}