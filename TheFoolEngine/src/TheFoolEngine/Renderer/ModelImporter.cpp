#include "tfpch.h"
#include "ModelImporter.h"

#include <fstream>
#include <stb_image.h>
#include <filesystem>

#include "UTF8IOSystem.h"

namespace TheFoolEngine
{
    
	ModelImporter::ModelImporter(const std::string& path, bool gamma)
		:m_GammaCorrection(gamma)
	{
		LoadModel(path);

		Assimp::Importer importer;
		bool hasFBX = importer.IsExtensionSupported(".fbx");
		TF_CORE_INFO("Assimp FBX support: {0}", hasFBX);

		// merge textures
		for (auto& mesh : m_MeshData)
			mesh->IntegrateTextures();
	}

	ModelImporter::~ModelImporter()
	{
	}

	void ModelImporter::LoadModel(const std::string& path)
	{
		TF_PROFILE_FUNCTION();
		// read file via ASSIMP
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			path, 
			aiProcess_Triangulate 
			| aiProcess_FlipUVs 
			| aiProcess_CalcTangentSpace 
			// | aiProcess_GenNormals
		);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)	// if is Not Zero
		{
			TF_CORE_ERROR("Error:: Assimp:: {0}", (void*)(importer.GetErrorString()));
			return;
		}

		m_Directory = path.substr(0, path.find_last_of('/'));

		std::string filetype = path.substr(path.find_last_of('.'));
		if (filetype.find("obj") != std::string::npos)
			m_FileType = FileType::OBJ;
		else if (filetype.find("glb") != std::string::npos)
			m_FileType = FileType::GLB;
		else if (filetype.find("fbx") != std::string::npos)
			m_FileType = FileType::FBX;
		else
		{
			m_FileType = FileType::UnSupported;
			TF_CORE_ERROR("UnSupported Model File Type.File Path = {0}", path);
			return;
		}

		ProcessNode(scene->mRootNode, scene);
	}

	void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_MeshData.push_back(ProcessMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i)
			ProcessNode(node->mChildren[i], scene);
	}

	Ref<MeshData> ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		// data to fill
		std::vector<MeshVertexData> vertices;
		std::vector<uint32_t> indices;
		std::vector<Ref<Texture2D>> textures;

		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			MeshVertexData vertex;

			// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			// position
			vertex.Position = glm::vec3(
				mesh->mVertices[i].x,
				mesh->mVertices[i].y, 
				mesh->mVertices[i].z 
			);	

			// normals
			if (mesh->HasNormals())
			{
				vertex.Normal = glm::vec3(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				);
			}

			// texture coordinates
			if (mesh->mTextureCoords[0])
			{
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).

				vertex.TexCoords = glm::vec2(
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
				);

				// tangent
				vertex.Tangent = glm::vec3(
					mesh->mTangents[i].x,
					mesh->mTangents[i].y,
					mesh->mTangents[i].z
				);

				// bitangent
				vertex.Bitangent = glm::vec3(
					mesh->mBitangents[i].x,
					mesh->mBitangents[i].y,
					mesh->mBitangents[i].z
				);
			}
			else
				vertex.TexCoords = glm::vec2(0.0f);

			vertices.push_back(vertex);
		}

		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];

			for (uint32_t j = 0; j < face.mNumIndices; ++j)
				indices.push_back(face.mIndices[j]);
		}

		// Create Material Builder
		auto builder = CreateRef<MaterialBuilder>();

		// Load Texuture
		aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
		
		SelectProcessMethod(aiMat, scene, builder, m_FileType);

		// Build Material
		auto material = builder->Build();

		// Create MeshData
		auto meshData = CreateRef<MeshData>(vertices, indices);
		meshData->SetMaterialID(MaterialManager::Get().GetIDByHash(material->GetHash()));

		TF_CORE_INFO("Processed mesh: {0} vertices, {1} indices, material: {2}",
			vertices.size(), indices.size(), material->GetHash());

		TF_CORE_INFO("Material {0}: Diffuse textures {1}, Normal textures {2}",
			material->GetHash(),
			aiMat->GetTextureCount(aiTextureType_DIFFUSE),
			aiMat->GetTextureCount(aiTextureType_NORMALS));

        {
            for (int t = 0; t <= AI_TEXTURE_TYPE_MAX; ++t) 
            {
                auto count = aiMat->GetTextureCount((aiTextureType)t);
                if (count > 0)
                    TF_CORE_INFO("  TextureType {0}: count={1}", t, count);
            }
        }

		return meshData;
	}

	TextureType ModelImporter::TranslateMeshTextureType(const aiTextureType& aitype)
	{
		switch (aitype)
		{
		case aiTextureType_NONE:
			return TextureType::None;
		case aiTextureType_DIFFUSE:
			return TextureType::Diffuse;
		case aiTextureType_SPECULAR:
			return TextureType::Specular;
		case aiTextureType_AMBIENT:
			return TextureType::Ambient;
		case aiTextureType_EMISSIVE:
			return TextureType::Emissive;
		case aiTextureType_HEIGHT:
			return TextureType::Height;
		case aiTextureType_NORMALS:
		// case aiTextureType_NORMAL_CAMERA:
			return TextureType::Normal;
		case aiTextureType_SHININESS:
			return TextureType::Shininess;
		case aiTextureType_OPACITY:
			return TextureType::Opacity;
		case aiTextureType_DISPLACEMENT:
			return TextureType::Displacement;
		case aiTextureType_LIGHTMAP:
			return TextureType::Lightmap;
		case aiTextureType_REFLECTION:
			return TextureType::Reflection;
		default:
			return TextureType::Unknown;
		}

		return TextureType::Error;
	}

	void ModelImporter::LoadTextureFromPath(aiMaterial* mat, aiTextureType aiType, Ref<MaterialBuilder>& builder)
	{
		TF_PROFILE_FUNCTION();

		auto type = TranslateMeshTextureType(aiType);

		for (uint32_t i = 0; i < mat->GetTextureCount(aiType); ++i)
		{
			aiString str;
			mat->GetTexture(aiType, i, &str);

            const auto* bytes = reinterpret_cast<const uint8_t*>(str.C_Str());
            std::string hexStr;
            for (int i = 0; i < str.length; ++i)
                hexStr += fmt::format("{:02x} ", bytes[i]);
            TF_CORE_INFO("Raw texture path hex ({0} bytes): {1}", str.length, hexStr);


            std::string raw = str.C_Str();

            std::stringstream ss;

            for (unsigned char c : raw)
            {
                ss << std::hex
                    << std::setw(2)
                    << std::setfill('0')
                    << (int)c
                    << " ";
            }

            TF_CORE_INFO("HEX = {0}", ss.str());

            int wlen = MultiByteToWideChar(
                CP_UTF8,
                0,
                str.C_Str(),
                -1,
                nullptr,
                0
            );

            std::wstring wstr(wlen, L'\0');

            MultiByteToWideChar(
                CP_UTF8,
                0,
                str.C_Str(),
                -1,
                wstr.data(),
                wlen
            );

            std::filesystem::path relative(wstr);

            std::filesystem::path dir =
                std::filesystem::u8path(m_Directory).make_preferred();

            std::filesystem::path fullpath = dir / relative;

            std::string path8 = fullpath.u8string();
            std::u16string path16 = fullpath.u16string();
            std::u32string path32 = fullpath.u32string();

			std::string key = std::to_string((uint8_t)type) + ":" + fullpath.u8string();

			auto it = m_TextureCache.find(key);
			if (it == m_TextureCache.end())
			{
				auto node = CreateRef<TextureCacheNode>();
				node->texture = Texture2D::Create(fullpath.u32string());
				node->id = node->texture->GetRendererID();
                node->path = fullpath.u8string();
				node->type = type;

				m_TextureCache[key] = node;

				builder->AddTexture(type, node->texture);
			}
			else
				builder->AddTexture(type, it->second->texture);
		}

		{
			aiString name;
			mat->Get(AI_MATKEY_NAME, name);
			TF_CORE_INFO("Mesh material: {0}", name.C_Str());
			// 各类型贴图数量 + 路径
			for (int t = 0; t <= AI_TEXTURE_TYPE_MAX; ++t) {
				auto ty = (aiTextureType)t;
				uint32_t n = mat->GetTextureCount(ty);
				if (n == 0) continue;
				for (uint32_t i = 0; i < n; ++i) {
					aiString path;
					mat->GetTexture(ty, i, &path);
					TF_CORE_INFO("  type={0} path={1}", t, path.C_Str());
				}
			}
			// glTF 常见：基色可能在 PBR 键上（Assimp 5+）
			aiString pbrPath;
			if (mat->GetTexture(aiTextureType_BASE_COLOR, 0, &pbrPath) == AI_SUCCESS)
				TF_CORE_INFO("  BASE_COLOR path={0}", pbrPath.C_Str());
		}

	}

	void ModelImporter::LoadTextureFromScene(aiMaterial* mat, aiTextureType aiType, const aiScene* scene, Ref<MaterialBuilder>& builder)
	{
		auto type = TranslateMeshTextureType(aiType);

		for (uint32_t i = 0; i < mat->GetTextureCount(aiType); ++i)
		{
			aiString str;
			mat->GetTexture(aiType, i, &str);

			std::string embeddedIndexStr = str.C_Str() + 1;
			int embeddedIndex = std::stoi(embeddedIndexStr);

			std::string key = std::to_string((uint8_t)type) + ":embedded_" + std::to_string(embeddedIndex);

			auto it = m_TextureCache.find(key);
			if (it == m_TextureCache.end())
			{
				auto texture = LoadEmbeddedTexture(scene, embeddedIndex);
				if (texture)
				{
					auto node = CreateRef<TextureCacheNode>();
					node->texture = texture;
					node->id = texture->GetRendererID();
					node->path = "embedded:" + std::to_string(embeddedIndex);
					node->type = type;

					m_TextureCache[key] = node;
					TF_CORE_INFO("Loaded embedded texture: index={0}, type={1}", embeddedIndex, static_cast<int>(type));

					builder->AddTexture(type, texture);
				}
				else 
					TF_CORE_ERROR("Loaded embedded texture Failed. EmbeddedIndex = {0}, type = {1}", embeddedIndex, (uint8_t)type);
			}
			else
				builder->AddTexture(type, it->second->texture);
		}

	}

	// TODO: 
	Ref<Texture2D> ModelImporter::LoadEmbeddedTexture(const aiScene* scene, int embeddedIndex)
	{
		if (!scene || !scene->HasTextures() || embeddedIndex >= (int)scene->mNumTextures)
		{
			TF_CORE_ERROR("Invalid embedded texture index: {0}", embeddedIndex);

			// white texture
			auto whiteTexture = Texture2D::Create(1, 1);
			uint32_t texData = 0xffffffff;
			whiteTexture->SetData(&texData, sizeof(uint32_t));

			return whiteTexture;
		}

		aiTexture* embeddedTexture = scene->mTextures[embeddedIndex];

		// check texture type
		if (embeddedTexture->mHeight == 0)
		{
			// embedded texture type. like: jreg | png
			return LoadCompressedEmbeddedTexture(embeddedTexture);
		}
		else
		{
			// Unembedded texture data
			return LoadUncompressedEmbeddedTexture(embeddedTexture);
		}
	}

	// TODO: process compressed embedded texture
	Ref<Texture2D> ModelImporter::LoadCompressedEmbeddedTexture(aiTexture* texture)
	{
		//unsigned char* buffer = new unsigned char[sizeof(&texture->pcData)];

		//memset(buffer, 0, texture->mWidth * 4);
		//for (int i = 0; i < texture->mWidth; ++i)
		//{
		//	int index = i * 4;

		//	buffer[index + 0] = texture->pcData[i].r;
		//	buffer[index + 1] = texture->pcData[i].g;
		//	buffer[index + 2] = texture->pcData[i].b;
		//	buffer[index + 3] = texture->pcData[i].a;
		//}

		std::string filetype = texture->achFormatHint;
		std::stringstream ss;
		ss << m_Directory << "/" << std::to_string(texture->mWidth * 4) << "." << filetype;

		std::string name = ss.str();

		std::ofstream file(ss.str(), std::ios::binary);

		if (file.is_open())
		{
			file.write((char*)texture->pcData, texture->mWidth);
			file.close();
		}

		// delete[] buffer;

		// return Texture2D::Create(texture->pcData, texture->mWidth);

		int width, height, channels;
		unsigned char* data = stbi_load_from_memory(
			(unsigned char*)texture->pcData,
			texture->mWidth,
			&width,
			&height,
			&channels,
			4
		);

		if (!data)
		{
			TF_CORE_ERROR("Failed to decode embedded texture.");
			auto whiteTex = Texture2D::Create(1, 1);
			uint32_t white = 0xffffffff;
			whiteTex->SetData(&white, sizeof(uint32_t));
			return whiteTex;
		}

		auto tex = Texture2D::Create(width, height);
		tex->SetData(data, width * height * 4);

		stbi_image_free(data);

		return tex;
	}

	Ref<Texture2D> ModelImporter::LoadUncompressedEmbeddedTexture(aiTexture* texture)
	{
		auto width = texture->mWidth;
		auto height = texture->mHeight;

		aiTexel* data = texture->pcData;

		auto resTexture = Texture2D::Create(width, height);
		resTexture->SetData(data, sizeof(aiTexel));

		auto name = texture->mFilename;

		return resTexture;
	}

	void ModelImporter::SelectProcessMethod(aiMaterial* mat, const aiScene* scene, Ref<MaterialBuilder>& builder, FileType type)
	{
		switch (type)
		{
			case FileType::OBJ:	ProcessOBJFile(mat, builder);	break;
			case FileType::GLB:	ProcessGLBFile(mat, scene, builder);	break;
			case FileType::FBX: ProcessFBXFile(mat, scene, builder); break;
		}
	}

	void ModelImporter::ProcessOBJFile(aiMaterial* mat, Ref<MaterialBuilder>& builder)
	{
		LoadTextureFromPath(mat, aiTextureType_DIFFUSE, builder);
		LoadTextureFromPath(mat, aiTextureType_NORMALS, builder);
		LoadTextureFromPath(mat, aiTextureType_SPECULAR, builder);
		LoadTextureFromPath(mat, aiTextureType_EMISSIVE, builder);
		LoadTextureFromPath(mat, aiTextureType_OPACITY, builder);
		LoadTextureFromPath(mat, aiTextureType_HEIGHT, builder);
		LoadTextureFromPath(mat, aiTextureType_AMBIENT, builder);
		LoadTextureFromPath(mat, aiTextureType_SHININESS, builder);
		LoadTextureFromPath(mat, aiTextureType_DISPLACEMENT, builder);
		LoadTextureFromPath(mat, aiTextureType_LIGHTMAP, builder);
		LoadTextureFromPath(mat, aiTextureType_REFLECTION, builder);
	}

	void ModelImporter::ProcessGLBFile(aiMaterial* mat, const aiScene* scene, Ref<MaterialBuilder>& builder)
	{
		LoadTextureFromScene(mat, aiTextureType_DIFFUSE, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_NORMALS, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_SPECULAR, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_EMISSIVE, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_OPACITY, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_HEIGHT, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_AMBIENT, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_SHININESS, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_DISPLACEMENT, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_LIGHTMAP, scene, builder);
		LoadTextureFromScene(mat, aiTextureType_REFLECTION, scene, builder);
	}

	void ModelImporter::ProcessFBXFile(aiMaterial* mat, const aiScene* scene, Ref<MaterialBuilder>& builder)
	{
		std::vector<aiTextureType> textureTypes = {
			aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_SPECULAR,
			aiTextureType_EMISSIVE, aiTextureType_OPACITY, aiTextureType_HEIGHT,
			aiTextureType_AMBIENT, aiTextureType_SHININESS, aiTextureType_DISPLACEMENT,
			aiTextureType_LIGHTMAP, aiTextureType_REFLECTION
		};

		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
		TF_CORE_INFO("FBX diffuse path: '{0}'", str.C_Str());
		TF_CORE_INFO("scene embedded texture count: {0}", scene->mNumTextures);

		for (auto aiType : textureTypes)
		{
			auto type = TranslateMeshTextureType(aiType);
			bool textureLoaded = false;

			if (mat->GetTextureCount(aiType) > 0)
			{
				LoadTextureFromPath(mat, aiType, builder);
				textureLoaded = true;
			}

			if (!textureLoaded)
			{
				auto solidTex = CreateSolidColorTexture(mat, aiType);
				if (solidTex)
				{
					builder->AddTexture(type, solidTex);
					TF_CORE_INFO("FBX: Created solid color texture for type {0}", static_cast<int>(type));
				}
			}
		}
	}

	Ref<Texture2D> ModelImporter::CreateSolidColorTexture(aiMaterial* mat, aiTextureType aiType)
	{
		const char* pKey = nullptr;
		unsigned int type = 0;
		unsigned int index = 0;

		if (aiType == aiTextureType_SHININESS) {
			uint8_t val = 128; // 0.5 * 255
			uint32_t texData = val | (val << 8) | (val << 16) | 0xFF000000;
			auto tex = Texture2D::Create(1, 1);
			tex->SetData(&texData, sizeof(uint32_t));
			return tex;
		}
		if (aiType == aiTextureType_SPECULAR) {
			uint32_t texData = 0x000000FF; // RGBA，R=0
			auto tex = Texture2D::Create(1, 1);
			tex->SetData(&texData, sizeof(uint32_t));
			return tex;
		}


		switch (aiType)
		{
		case aiTextureType_DIFFUSE:
			pKey = AI_MATKEY_COLOR_DIFFUSE; break;
		case aiTextureType_SPECULAR:
			pKey = AI_MATKEY_COLOR_SPECULAR; break;
		case aiTextureType_AMBIENT:
			pKey = AI_MATKEY_COLOR_AMBIENT; break;
		case aiTextureType_EMISSIVE:
			pKey = AI_MATKEY_COLOR_EMISSIVE; break;
		case aiTextureType_SHININESS:
			return nullptr;
		case aiTextureType_OPACITY:
		{
			float opacity = 1.0f;
			if (AI_SUCCESS == mat->Get(AI_MATKEY_OPACITY, opacity))
			{
				uint8_t alpha = static_cast<uint8_t>(opacity * 255);
				uint32_t texData = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
				auto tex = Texture2D::Create(1, 1);
				tex->SetData(&texData, sizeof(uint32_t));
				return tex;
			}
		}
		return nullptr;
		default:
			return nullptr;
		}

		aiColor3D color(1.0f, 1.0f, 1.0f);
		if (AI_SUCCESS == mat->Get(pKey, type, index, color))
		{
			uint8_t r = static_cast<uint8_t>(color.r * 255);
			uint8_t g = static_cast<uint8_t>(color.g * 255);
			uint8_t b = static_cast<uint8_t>(color.b * 255);
			uint32_t texData = (r << 16) | (g << 8) | (b << 0) | 0xFF000000; // alpha = 255
			auto tex = Texture2D::Create(1, 1);
			tex->SetData(&texData, sizeof(uint32_t));
			return tex;
		}

		return nullptr;
	}

}
