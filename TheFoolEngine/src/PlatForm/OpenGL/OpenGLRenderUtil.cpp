#include "tfpch.h"
#include "OpenGLRenderUtil.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine 
{

    static const glm::mat4 s_CaptureViews[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3(0.0f,  0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3(0.0f,  0.0f,-1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    };

    void OpenGLRenderUtil::SaveViewport(int prev[4])
    {
        glGetIntegerv(GL_VIEWPORT, prev);
    }

    void OpenGLRenderUtil::RestoreViewport(const int prev[4])
    {
        glViewport(prev[0], prev[1], prev[2], prev[3]);
    }

    Ref<VertexArray> OpenGLRenderUtil::CreateCubeVAO()
    {
        float verts[] = {
            -1, 1,-1, -1,-1,-1,  1,-1,-1,
             1,-1,-1,  1, 1,-1, -1, 1,-1,
            -1,-1, 1, -1,-1,-1, -1, 1,-1,
            -1, 1,-1, -1, 1, 1, -1,-1, 1,
             1,-1,-1,  1,-1, 1,  1, 1, 1,
             1, 1, 1,  1, 1,-1,  1,-1,-1,
            -1,-1, 1, -1, 1, 1,  1, 1, 1,
             1, 1, 1,  1,-1, 1, -1,-1, 1,
            -1, 1,-1,  1, 1,-1,  1, 1, 1,
             1, 1, 1, -1, 1, 1, -1, 1,-1,
            -1,-1,-1, -1,-1, 1,  1,-1,-1,
             1,-1,-1, -1,-1, 1,  1,-1, 1,
        };

        auto vao = VertexArray::Create();
        auto vbo = VertexBuffer::Create(verts, sizeof(verts));
        vbo->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
        vao->AddVertexBuffer(vbo);
        return vao;
    }

    Ref<VertexArray> OpenGLRenderUtil::CreateFullscreenQuadVAO()
    {
        float verts[] = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
        };
        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

        auto vao = VertexArray::Create();
        auto vbo = VertexBuffer::Create(verts, sizeof(verts));
        vbo->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
        vao->AddVertexBuffer(vbo);

        auto ibo = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        vao->SetIndexBuffer(ibo);
        return vao;
    }

    void OpenGLRenderUtil::RenderToCubemapFaces(
        Ref<CubeMap> dst, uint32_t faceSize,
        Ref<Shader> shader,
        const glm::mat4& projection,
        std::function<void(int face, const glm::mat4& view)> bindUniforms,
        int mipLevel)
    {
        int prevViewport[4];
        SaveViewport(prevViewport);

        GLuint fbo;
        glCreateFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glDisable(GL_DEPTH_TEST);
        uint32_t mipSize = faceSize >> mipLevel;
        glViewport(0, 0, mipSize, mipSize);

        shader->Bind();
        shader->SetMat4("u_Projection", projection);

        auto cubeVAO = CreateCubeVAO();
        cubeVAO->Bind();

        for (int face = 0; face < 6; ++face)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, dst->GetRendererID(), mipLevel);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            bindUniforms(face, s_CaptureViews[face]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);

        RestoreViewport(prevViewport);
    }

    void OpenGLRenderUtil::RenderToTexture2D(Ref<Texture2D> texture, uint32_t size, Ref<Shader> shader, std::function<void()> bindUniforms)
    {
        auto vao = RenderUtil::Get()->CreateFullscreenQuadVAO();

        GLint prevViewport[4];
        glGetIntegerv(GL_VIEWPORT, prevViewport);

        GLuint fbo;
        glCreateFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texture->GetRendererID(), 0);

        glViewport(0, 0, size, size);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->Bind();
        vao->Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);

        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    }

}
