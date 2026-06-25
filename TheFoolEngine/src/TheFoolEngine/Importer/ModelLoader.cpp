#include "tfpch.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

namespace TheFoolEngine
{
    namespace
    {
        Ref<Texture2D> LoadTexture(aiMaterial* mat, aiTextureType aiType, const aiScene* scene, const std::filesystem::path& modelDir)
        {
            if (mat->GetTextureCount(aiType) == 0)
                return nullptr;

            aiString str;
            mat->GetTexture(aiType, 0, &str);
            std::string pathStr = str.C_Str();

            // embedded texture
            if (pathStr[0] == '*')
            {
                int index = std::stoi(pathStr.substr(1));
                if (!scene->HasTextures() || index >= (int)scene->mNumTextures)
                    return nullptr;

                aiTexture* tex = scene->mTextures[index];
                int w, h, c;
                unsigned char* data = stbi_load_from_memory((unsigned char*)tex->pcData, tex->mWidth, &w, &h, &c, 4);
                if (!data) 
                    return nullptr;

                auto result = Texture2D::Create(w, h);
                result->SetData(data, w * h * 4);
                stbi_image_free(data);
                return result;
            }

            // external texture
            std::filesystem::path fullPath = modelDir / pathStr;
            return Texture2D::Create(fullPath);
        }

        PBRMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene)
        {
            PBRMeshData meshData;
            meshData.MaterialIndex = mesh->mMaterialIndex;

            // vertices
            meshData.vertices.resize(mesh->mNumVertices);
            for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
            {
                auto& vertex = meshData.vertices[i];

                // position
                vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                // normal
                if (mesh->HasNormals())
                    vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                // UV
                if (mesh->mTextureCoords[0])
                {
                    vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                    vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                    vertex.bitTangent = { mesh->mBitangents[i].x,mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                }
                // bone
                memset(vertex.boneIDs, 0, sizeof(vertex.boneIDs));
                memset(vertex.weights, 0, sizeof(vertex.weights));
            }

            // indices
            for (uint32_t f = 0; f < mesh->mNumFaces; ++f)
            {
                aiFace& face = mesh->mFaces[f];
                for (uint32_t j = 0;j < face.mNumIndices; ++j)
                    meshData.indices.push_back(face.mIndices[j]);
            }

            return meshData;
        }

        void ProcessNode(aiNode* node, const aiScene* scene, std::vector<PBRMeshData>& meshes)
        {
            for (uint32_t i = 0; i < node->mNumMeshes; ++i)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(std::move(ProcessMesh(mesh, scene)));
                aiMatrix4x4 m = node->mTransformation;
                meshes.back().NodeTransform = glm::transpose(glm::make_mat4(&m.a1));
            }
            for (uint32_t i = 0; i < node->mNumChildren; ++i)
                ProcessNode(node->mChildren[i], scene, meshes);
        }

        PBRMaterialTextureSet ProcessMaterial(aiMaterial* mat, const aiScene* scene, const std::filesystem::path& modelDir)
        {
            PBRMaterialTextureSet textureSet;

            // map
            textureSet.AlbedoMap = LoadTexture(mat, aiTextureType_DIFFUSE, scene, modelDir);
            if (!textureSet.AlbedoMap)
                textureSet.AlbedoMap = LoadTexture(mat, aiTextureType_BASE_COLOR, scene, modelDir);

            textureSet.NormalMap = LoadTexture(mat, aiTextureType_NORMALS, scene, modelDir);
            textureSet.MetallicRoughnessMap = LoadTexture(mat, aiTextureType_METALNESS, scene, modelDir);

            // factor
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, textureSet.AlbedoFactor);
            mat->Get(AI_MATKEY_METALLIC_FACTOR, textureSet.MetallicFactor);
            mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, textureSet.RoughnessFactor);

            // AO
            textureSet.AOMap = LoadTexture(mat, aiTextureType_LIGHTMAP, scene, modelDir);
            if (!textureSet.AOMap)
                textureSet.AOMap = LoadTexture(mat, aiTextureType_AMBIENT, scene, modelDir);

            return textureSet;
        }
    }

    PBRMaterialData AssimpImporter::Import(const std::filesystem::path& path)
    {
        static constexpr aiPostProcessSteps s_AssimpLoadType[] = {
            aiProcess_Triangulate,
            aiProcess_FlipUVs,
            aiProcess_CalcTangentSpace,
        };

        unsigned int flags = 0;
        for (auto flag : s_AssimpLoadType)
            flags |= (unsigned int)flag;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string().c_str(), flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            TF_CORE_ERROR("Error:: Assimp:: {0}", (void*)(importer.GetErrorString()));
            return {};
        }

        std::vector<PBRMeshData> meshes;
        ProcessNode(scene->mRootNode, scene, meshes);

        std::filesystem::path modelDir = path.parent_path();

        std::vector<PBRMaterialTextureSet> textures;
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            aiMaterial* mat = scene->mMaterials[i];
            textures.push_back(std::move(ProcessMaterial(mat, scene, modelDir)));
        }

        PBRMaterialData result;
        result.Meshes = std::move(meshes);
        result.Textures = std::move(textures);
        return result;
    }
}