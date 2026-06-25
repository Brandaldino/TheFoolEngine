#include "tfpch.h"

#include "PBRModel.h"
#include "ModelLoader.h"

#include "../Renderer/RendererAPI.h"

namespace TheFoolEngine
{
    PBRModel::PBRModel()
    {
    }

    PBRModel::~PBRModel()
    {
        Release();
    }

    void PBRModel::Import(std::filesystem::path& path)
    {
        PBRMaterialData data = AssimpImporter::Import(path);
        
        m_ModelData = data;
    }

    void PBRModel::UpLoad()
    {
        if (m_ModelData.Meshes.empty() || m_ModelData.Textures.empty())
            return;

        // mesh
        for (auto& mesh : m_ModelData.Meshes)
        {
            Ref<VertexArray> vao = VertexArray::Create();

            uint32_t vboSize = mesh.vertices.size() * sizeof(PBRVertexData);
            uint32_t iboSize = mesh.indices.size() * sizeof(uint32_t);

            Ref<VertexBuffer> vbo = VertexBuffer::Create(vboSize);
            vbo->SetLayout(
                {
                    { ShaderDataType::Float3,   "a_Position"  },
                    { ShaderDataType::Float2,   "a_TexCoord"  },
                    { ShaderDataType::Float3,   "a_Tangent"   },
                    { ShaderDataType::Float3,   "a_BitTangent"},
                    { ShaderDataType::Float3,   "a_Normal"    },
                    { ShaderDataType::Int4,     "a_BoneIDs"   },
                    { ShaderDataType::Float4,   "a_Weights"   }
                }
            );
            vbo->SetData(mesh.vertices.data(), vboSize);

            Ref<IndexBuffer> ibo = IndexBuffer::Create(iboSize);
            ibo->SetData(mesh.indices.data(), iboSize);

            vao->AddVertexBuffer(vbo);
            vao->SetIndexBuffer(ibo);

            // ensure data alive
            m_VertexArray.push_back(vao);
        }
    }

    void PBRModel::Release()
    {
        if (!m_VertexArray.empty())
            m_VertexArray.clear();
    }

}
