#include "tfpch.h"
#include "AssetRegistry.h"

#include <filesystem>

namespace TheFoolEngine
{

    AssetRegistry& AssetRegistry::Get()
    {
        static AssetRegistry instance;
        return instance;
    }

    UUID AssetRegistry::Register(Ref<Asset> asset, const std::string& path)
    {
        std::string key = NormalizePath(path);
        auto it = m_PathToID.find(key);
        if (it != m_PathToID.end())
            return it->second;  // Deduplicate by path → return existing ID

        m_Assets[asset->GetID()] = asset;
        m_PathToID[key] = asset->GetID();
        return asset->GetID();
    }

    Ref<Asset> AssetRegistry::GetAsset(UUID id) const
    {
        auto it = m_Assets.find(id);
        return it != m_Assets.end() ? it->second : nullptr;
    }

    UUID AssetRegistry::GetIDByPath(const std::string& path) const
    {
        auto it = m_PathToID.find(NormalizePath(path));
        return it != m_PathToID.end() ? it->second : UUID{ 0 };
    }

    Ref<Asset> AssetRegistry::GetByPath(const std::string& path) const
    {
        UUID id = GetIDByPath(NormalizePath(path));
        return id != UUID{ 0 } ? GetAsset(id) : nullptr;
    }

    bool AssetRegistry::HasPath(const std::string& path) const
    {
        return m_PathToID.count(NormalizePath(path)) > 0;
    }

    std::string AssetRegistry::NormalizePath(const std::string& path)
    {
        return std::filesystem::weakly_canonical(path).string();
    }

    UUID AssetRegistry::HashPath(const std::string& path)
    {
        return std::hash<std::string>{}(NormalizePath(path));
    }

}
