#pragma once

#include "TheFoolEngine/Core/UUID.h"
#include "TheFoolEngine/Core/Base.h"

namespace TheFoolEngine
{
    class PBRModel;

    enum class AssetType
    {
        Model,
        Texture,
        Material
    };

    class Asset
    {
    public:
        virtual ~Asset() = default;

        UUID GetID() const { return m_UUID; };
        const std::string& GetPath() const { return m_Path; };
        AssetType GetType() const { return m_Type; };
    protected:
        Asset(UUID uuid, const std::string& path, AssetType type)
            : m_UUID(uuid), m_Path(path), m_Type(type) {
        }
    private:
        UUID m_UUID;
        std::string m_Path;
        AssetType m_Type;
    };

    class ModelAsset : public Asset
    {
    public:
        ModelAsset(const Ref<PBRModel>& model, UUID id, const std::string& path)
            : Asset(id, path, AssetType::Model), m_Model(model) {
        }

        Ref<PBRModel> GetModel() const { return m_Model; };
    private:
        Ref<PBRModel> m_Model;
    };

}