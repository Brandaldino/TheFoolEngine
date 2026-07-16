#include "tfpch.h"
#include "OpenGLCubeMap.h"

#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>

#include "../../TheFoolEngine/Renderer/Shader.h"

namespace TheFoolEngine
{
    OpenGLCubeMap::OpenGLCubeMap(uint32_t size)
        : m_Path("")
    {
        TF_PROFILE_FUNCTION();

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, GL_RGB16F, size, size);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    OpenGLCubeMap::OpenGLCubeMap(const std::filesystem::path& path)
        :m_Path(path.u8string())
    {
        TF_PROFILE_FUNCTION();

        uint32_t whitePixel = 0xFFFFFFFF;
        int width, height, channels;
        float* data = nullptr;

        std::wstring wpath = path.wstring();

        FILE* fp = _wfopen(wpath.c_str(), L"rb");
        if (!fp)
        {
            TF_CORE_ERROR("Failed to load texture from path: {0}", path.u8string());
            return;
        }

        fseek(fp, 0, SEEK_END);
        long fileBytesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<stbi_uc> buffer(fileBytesize);

        fread(buffer.data(), 1, fileBytesize, fp);
        fclose(fp);
        stbi_set_flip_vertically_on_load(1);
        {
            TF_PROFILE_SCOPE("OpenGLCubeMap::OpenGLCubeMap(const std::filesystem::path& path)");
            width = 1;
            height = 1;
            channels = 4;
            data = stbi_loadf_from_memory(buffer.data(), (int)fileBytesize, &width, &height, &channels, 0);
        }

        if (!data)
        {
            TF_CORE_ERROR("Failed to load texture: {0}. Falling back to 1x1 white.", path.string());
            width = 1;
            height = 1;
            channels = 4;
            data = (float*)&whitePixel;
        }

        int faceSize = height;

        // upload HDR as a temporary 2D texture
        GLuint equirectTex;
        glCreateTextures(GL_TEXTURE_2D, 1, &equirectTex);
        glTextureStorage2D(equirectTex, 1, GL_RGB16F, width, height);
        glTextureSubImage2D(equirectTex, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);
        glTextureParameteri(equirectTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(equirectTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(equirectTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(equirectTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(equirectTex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // create an empty cubemap
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, GL_RGB16F, faceSize, faceSize);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // cube ( 36 vertices, just position)
        float cubeVerts[] = {
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

        GLuint cubeVAO, cubeVBO;
        glCreateVertexArrays(1, &cubeVAO);
        glCreateBuffers(1, &cubeVBO);
        glNamedBufferStorage(cubeVBO, sizeof(cubeVerts), cubeVerts, 0);
        glVertexArrayVertexBuffer(cubeVAO, 0, cubeVBO, 0, 12);
        glEnableVertexArrayAttrib(cubeVAO, 0);
        glVertexArrayAttribFormat(cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(cubeVAO, 0, 0);

        // Shader + Matrix
        auto shader = Shader::Create("assets/shader/EquirectToCubeMap.glsl");

        glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        glm::mat4 views[6] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),glm::vec3(0.0f,-1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f)),
        };

        // FBO + six times renderer
        GLint prevViewport[4];
        glGetIntegerv(GL_VIEWPORT, prevViewport);

        GLuint fbo;
        glCreateFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, faceSize, faceSize);

        shader->Bind();
        shader->SetMat4("u_Projection", proj);
        shader->SetInt("u_EquirectMap", 0);
        glBindTextureUnit(0, equirectTex);

        for (int face = 0; face < 6; ++face)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_RendererID, 0);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader->SetMat4("u_View", views[face]);

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDeleteFramebuffers(1, &fbo);
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteTextures(1, &equirectTex);

        if (data != (float*)&whitePixel)
            stbi_image_free(data);
    }

    OpenGLCubeMap::~OpenGLCubeMap()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLCubeMap::Bind(uint32_t slot) const
    {
        TF_PROFILE_FUNCTION();

        glBindTextureUnit(slot, m_RendererID);
    }

    void OpenGLCubeMap::GenerateMipmap()
    {
        glGenerateTextureMipmap(m_RendererID);
    }
    
    // === CubeMap ========================================================
    std::unordered_map<std::filesystem::path, Ref<CubeMap>> CubeMapCache::s_CubeMapCache;
    bool CubeMapCache::Regist(const std::filesystem::path& path, Ref<CubeMap> cubemap)
    {
        if (s_CubeMapCache.find(path) != s_CubeMapCache.end())
            return false;

        s_CubeMapCache[path] = cubemap;
        return true;
    }

    Ref<CubeMap> CubeMapCache::FindCubeMap(const std::filesystem::path& path)
    {
        if (s_CubeMapCache.find(path) != s_CubeMapCache.end())
            return s_CubeMapCache[path];

        return nullptr;
    }

    void CubeMapCache::Clear()
    {
        s_CubeMapCache.clear();
    }

}