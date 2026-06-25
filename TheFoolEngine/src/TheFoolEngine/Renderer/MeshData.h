#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Material.h"

#include "TheFoolEngine/Core/Base.h"
#include "Texture.h"


namespace TheFoolEngine
{

#define MAX_BONE_INFLUENCE 4

	struct MeshVertexData
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
		int BoneIDs[MAX_BONE_INFLUENCE];		//bone indexes which will influence this vertex
		float Weights[MAX_BONE_INFLUENCE];		//weights from each bone
	};

	struct SubTexture
	{
		TextureType Type;
		Ref<Texture2D> Texture;
		glm::vec2 Position;
		glm::vec2 Size;

		glm::vec2 UVOffset = glm::vec2(0.0f);
		glm::vec2 UVScale = glm::vec2(1.0f);
	};

	struct SubTextureInfo
	{
		TextureType Type;
		Ref<Texture2D> AtlasTexture;
		glm::vec2 AtlasSize;
	};

	class MeshData
	{
	public:
		MeshData(const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices);
		~MeshData();

		const std::vector<MeshVertexData>& GetVertexArray() { return m_Vertices; }
		const std::vector<uint32_t>& GetIndexArray() { return m_Indices; }

		void AddTexture(const TextureType& type, const Ref<Texture2D>& texture);
		std::vector<Ref<Texture2D>> GetTexture(const TextureType& type) const;

		void IntegrateTextures();	// combine textures of the same type into one.

		void SetMaterialID(uint32_t id) { m_MaterialID = id; }
		uint32_t GetMaterialID() { return m_MaterialID; }
	private:
		glm::vec2 AnalyzeScaleLevel(const std::vector<Ref<Texture2D>>& data); // average size
		void CalcRowAndColumn(unsigned int& row, unsigned int& column);
	private:
		std::vector<MeshVertexData> m_Vertices;
		std::vector<uint32_t> m_Indices;

		std::unordered_map<TextureType, std::vector<Ref<Texture2D>>> m_Textures;
		std::unordered_map<TextureType, Ref<SubTextureInfo>> m_SubTextures;

		uint32_t m_MaterialID;
	};

}
