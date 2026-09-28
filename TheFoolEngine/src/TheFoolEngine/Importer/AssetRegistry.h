#pragma once

#include "Asset.h"
#include "TheFoolEngine/Core/Base.h"

#include <unordered_map>

namespace TheFoolEngine
{
    class AssetRegistry
    {
    public:
        static AssetRegistry& Get();

        // Regist | Inquire
        UUID Register(Ref<Asset> asset, const std::string& path);
        Ref<Asset> GetAsset(UUID id) const;
        UUID GetIDByPath(const std::string& path) const;
        Ref<Asset> GetByPath(const std::string& path) const;
        bool HasPath(const std::string& path) const;

        static std::string NormalizePath(const std::string& path);
        static UUID HashPath(const std::string& path);
    private:
        std::unordered_map<UUID, Ref<Asset>> m_Assets;  // ID -> asset
        std::unordered_map<std::string, UUID> m_PathToID;   // path -> ID
    };
}