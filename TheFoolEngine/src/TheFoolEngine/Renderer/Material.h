#pragma once

#include <glm/glm.hpp>

#include "TheFoolEngine/Core/Base.h"
#include "Texture.h"
#include "Shader.h"

namespace TheFoolEngine
{
#define MATERIAL_PROPERTY(type, name) \
	void Set##name(type value){m_Info.name = value; } \
	type Get##name() const { return m_Info.name; }

#define MATERIAL_VEC_PROPERTY(type, name) \
	void Set##name(const type& value){m_Info.name = value; } \
	const type& Get##name() const { return m_Info.name; };

	enum class TextureType : uint8_t
	{
		// Core Texture Type
		WhiteTexture = 0,	// Default fallback texture
		Diffuse,			// Basic color
		Normal,
		Specular,
		Emissive,
		Opacity,
		Height,

		// On-demand support
		Ambient,			// Ambient light - Usually replaced by AO
		Shininess,			// Luster - Usually replaced by Roughness
		Displacement,		// Replacement - Similar to Height but even stronger
		Lightmap,
		Reflection,
		Unknown,

		None,
		Error
	};

	// ------------------ Material ------------------ //
	class Material : public std::enable_shared_from_this<Material>
	{
	public:
		Material(const std::unordered_map<TextureType, std::vector<Ref<Texture2D>>>& textures, uint32_t hash);
		~Material();

		const uint32_t& GetHash() { return m_Hash; }
		const std::unordered_map<TextureType, std::vector<Ref<Texture2D>>>& GetTextures() { return m_Textures; }
		const std::vector<Ref<Texture2D>>& GetTextureByType(TextureType type);
	private:
		uint32_t m_Hash;
		std::unordered_map<TextureType, std::vector<Ref<Texture2D>>> m_Textures;
	};

	// ------------------ Material Manager ------------------ //
	class MaterialManager
	{
	public:
		static MaterialManager& Get()
		{
			static MaterialManager instance;
			return instance;
		}

		uint32_t RegisterMaterial(const Ref<Material>& material);
		uint32_t GetIDByHash(uint32_t hash);
		uint32_t GetHashByID(uint32_t ID);
		Ref<Material> GetMaterial(const uint32_t& id);
		Ref<Material> GetMaterialByHash(const uint32_t& hash);
		void UnRegisterMaterial(uint32_t id);

		// uint32_t UpdateMaterial(const Ref<Material>& material);

		static uint32_t GenerateMaterialHash(const std::unordered_map<TextureType, std::vector<Ref<Texture2D>>>& textures);
	private:
		std::unordered_map<uint32_t, Ref<Material>> m_IDToMaterials;
		std::unordered_map<uint32_t, uint32_t> m_HashToID;
		std::unordered_map<uint32_t, uint32_t> m_IDToHash;

		std::atomic<uint32_t> m_NextID{ 1 };
		std::mutex m_Mutex;
	};

	// ------------------ Material Builder ------------------ //
	class MaterialBuilder
	{
	public:
		MaterialBuilder& AddTexture(TextureType type, const Ref<Texture2D>& texture);
		Ref<Material> Build();

		const std::vector<Ref<Texture2D>>& GetTextureByType(TextureType type)
		{
			auto it = m_Textures.find(type);
			return it != m_Textures.end() ? it->second : std::vector<Ref<Texture2D>>();
		}
	private:
		std::unordered_map<TextureType, std::vector<Ref<Texture2D>>> m_Textures;
	};
}
