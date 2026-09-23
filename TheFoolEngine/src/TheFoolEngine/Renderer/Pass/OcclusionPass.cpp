#include "tfpch.h"
#include "OcclusionPass.h"

#include "TheFoolEngine/Renderer/RenderCommand.h"
#include "TheFoolEngine/Renderer/Shader.h"
#include "TheFoolEngine/Renderer/RenderGraph.h"
#include "TheFoolEngine/Renderer/OcclusionManager.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{

    OcclusionPass::OcclusionPass(Ref<Shader> shader)
        : m_Shader(shader)
    {
        float verts[] = {
            -1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
            -1,-1, 1,  1,-1, 1,  1, 1, 1, -1, 1, 1,
        };
        uint32_t idx[] = {
            0,1,2, 0,2,3,  4,6,5, 4,7,6,  0,4,5, 0,5,1,
            1,5,6, 1,6,2,  2,6,7, 2,7,3,  3,7,4, 3,4,0
        };

        m_CubeVAO = VertexArray::Create();
        Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(verts));
        vbo->SetLayout({ {ShaderDataType::Float3, "a_Position"} });
        vbo->SetData(verts, sizeof(verts));
        m_CubeVAO->AddVertexBuffer(vbo);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(sizeof(idx));
        ibo->SetData(idx, sizeof(idx));
        m_CubeVAO->SetIndexBuffer(ibo);
    }

    std::vector<TextureHandle>& OcclusionPass::GetInputs()
    {
        m_Inputs = { m_DepthHandle };
        return m_Inputs;
    }

    std::vector<TextureHandle>& OcclusionPass::GetOutputs()
    {
        m_Outputs = {};
        return m_Outputs;
    }

    void OcclusionPass::Execute(RenderContext& context)
    {
        if (!context.Occlusion)
            return;

        auto fb = context.RenderGraph->GetFrameBuffer(m_DepthHandle);
        if (!fb)
            return;

        fb->Bind();
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::Off);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);    // no color

        m_Shader->Bind();
        m_Shader->SetMat4("u_Projection", context.Camera.ProjectionMatrix);
        m_Shader->SetMat4("u_View", context.Camera.ViewMatrix);

        for (auto id : context.Occlusion->GetEntitiesToTest())
        {
            auto& entry = context.Occlusion->GetEntry(id);

            glm::mat4 box = glm::translate(glm::mat4(1.0f), entry.BoundsCenter) * glm::scale(glm::mat4(1.0f), entry.BoundsHalfExtents);

            entry.Query->Begin();
            m_Shader->SetMat4("u_Model", box);
            m_CubeVAO->Bind();
            RenderCommand::DrawIndexed(m_CubeVAO, 36);
            entry.Query->End();
        }

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        fb->UnBind();
    }

    void OcclusionPass::SetDepthHandle(TextureHandle& handle)
    {
        m_DepthHandle = handle;
    }


}
