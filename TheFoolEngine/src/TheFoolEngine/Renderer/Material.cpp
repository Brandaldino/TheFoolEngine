#include "tfpch.h"
#include "Material.h"

#include <sstream>

namespace TheFoolEngine
{
	// ---------------- Material  ---------------- //
	Material::Material(const std::unordered_map<TextureType, std::vector<Ref<Texture2D>>>& textures, uint32_t hash)
		: m_Hash(hash)
	{
		m_Textures = textures;
	}

	Material::~Material()
	{
		// MaterialManager::Get().UnRegisterMaterial(m_Hash);
	}

	const std::vector<Ref<Texture2D>>& Material::GetTextureByType(TextureType type)
	{
		auto it = m_Textures.find(type);

		if (it != m_Textures.end())
			return it->second;
		else
			return std::vector<Ref<Texture2D>>();

		// return it != m_Textures.end() ? it->second : std::vector<Ref<Texture2D>>();
	}

	// ---------------- Material Manager ---------------- //
	uint32_t MaterialManager::RegisterMaterial(const Ref<Material>& material)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		uint32_t hash = material->GetHash();
		auto hashIt = m_HashToID.find(hash);
		if (hashIt != m_HashToID.end()) {
			return hashIt->second;
		}

		uint32_t id = m_NextID++;
		m_IDToMaterials[id] = material;
		m_HashToID[hash] = id;
		m_IDToHash[id] = hash;

		TF_CORE_INFO("Registered material: ID={0}, Hash={1}", id, hash);
		return id;
	}

	void MaterialManager::UnRegisterMaterial(uint32_t id)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		auto idIt = m_IDToMaterials.find(id);
		if (idIt != m_IDToMaterials.end()) 
		{
			uint32_t hash = m_IDToHash[id];

			TF_CORE_INFO("Unregistering material: ID={0}, Hash={1}", id, hash);

			m_IDToMaterials.erase(idIt);
			m_HashToID.erase(hash);
			m_IDToHash.erase(id);
		}
	}

	uint32_t MaterialManager::GetIDByHash(uint32_t hash)
	{
		auto it = m_HashToID.find(hash);
		if (it != m_HashToID.end())
			return it->second;
		
		TF_CORE_ASSERT(it != m_HashToID.end(), "Material Hash is not existed.");
	}

	uint32_t MaterialManager::GetHashByID(uint32_t id)
	{
		for (auto it : m_HashToID)
		{
			if (it.second == id)
				return it.first;
		}

		TF_CORE_ASSERT(false, "Material ID is not existed.");
		return 0;
	}

	Ref<Material> MaterialManager::GetMaterial(const uint32_t& id)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		auto it = m_IDToMaterials.find(id);
		return it != m_IDToMaterials.end() ? it->second : nullptr;
	}

	Ref<Material> MaterialManager::GetMaterialByHash(const uint32_t& hash)
	{
		auto it = m_HashToID.find(hash);
		if (it != m_HashToID.end())
			return m_IDToMaterials[it->second];

		return nullptr;
	}

	uint32_t MaterialManager::GenerateMaterialHash(const std::unordered_map<TextureType, std::vector<Ref<Texture2D>>>& textures)
	{
		std::stringstream ss;

		// Serialize texture information
		for (const auto& [type, group] : textures)
		{
			for (const auto& texture : group)
			{
				if (texture)
					ss << static_cast<uint8_t>(type) << texture->GetPath();
			}
		}

		return std::hash<std::string>{}(ss.str());
	}

	// ---------------- Material Builder ---------------- //
	MaterialBuilder& MaterialBuilder::AddTexture(TextureType type, const Ref<Texture2D>& texture)
	{
		m_Textures[type].push_back(texture);

		return *this;
	}

	Ref<Material> MaterialBuilder::Build()
	{
		// generate hash
		uint32_t hash = MaterialManager::Get().GenerateMaterialHash(m_Textures);

		// check whether material existed
		auto existing = MaterialManager::Get().GetMaterialByHash(hash);
		if (existing) {
			TF_CORE_INFO("Reusing existing material: {}", hash);
			return existing;
		}

		// create new material
		auto material = CreateRef<Material>(m_Textures, hash);

		// material to register
		MaterialManager::Get().RegisterMaterial(material);

		TF_CORE_INFO("Created new material: {}", hash);
		return material;
	}
}
