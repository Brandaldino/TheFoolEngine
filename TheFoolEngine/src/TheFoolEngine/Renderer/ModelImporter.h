#pragma once

#include "MeshData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace TheFoolEngine
{
	enum class FileType : uint8_t
	{
		UnSupported = 0,
		OBJ,
		GLB
	};

	struct TextureCacheNode
	{
		uint32_t id;
		std::string path;
		TextureType type;
		Ref<Texture2D> texture;
	};

	class ModelImporter
	{
	public:
		ModelImporter(const std::string& path, bool gamma = false);
		~ModelImporter();

		// static void ImportModel(const std::string& path, bool gamma = false);

		const std::vector<Ref<MeshData>>& GetMeshData() { return m_MeshData; }

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Ref<MeshData> ProcessMesh(aiMesh* mesh, const aiScene* scene);

		TextureType TranslateMeshTextureType(const aiTextureType& aitype);
		void LoadTextureFromPath(aiMaterial* mat, aiTextureType aiType, Ref<MaterialBuilder>& builder);
		void LoadTextureFromScene(aiMaterial* mat, aiTextureType aiType, const aiScene* scene, Ref<MaterialBuilder>& builder);	// for glb
		Ref<Texture2D> LoadEmbeddedTexture(const aiScene* scene, int embeddedIndex);

		Ref<Texture2D> LoadCompressedEmbeddedTexture(aiTexture* texture);
		Ref<Texture2D> LoadUncompressedEmbeddedTexture(aiTexture* texture);

		void SelectProcessMethod(aiMaterial* mat, const aiScene* scene, Ref<MaterialBuilder>& builder, FileType type);
		void ProcessOBJFile(aiMaterial* mat, Ref<MaterialBuilder>& builder);
		void ProcessGLBFile(aiMaterial* mat, const aiScene* scene, Ref<MaterialBuilder>& builder);
	private:
		std::string m_Directory;
		bool m_GammaCorrection;
		std::vector<Ref<MeshData>> m_MeshData;

		std::unordered_map<std::string, Ref<TextureCacheNode>> m_TextureCache;

		FileType m_FileType;
	};
}

