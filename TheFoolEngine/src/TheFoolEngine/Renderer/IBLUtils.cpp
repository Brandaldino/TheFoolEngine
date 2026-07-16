#include "tfpch.h"
#include "IBLUtils.h"

#include "Shader.h"
#include "VertexArray.h"
#include "Buffer.h"

#include "RenderUtil.h"

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{
    Ref<CubeMap> IBLUtils::CreateIrradianceMap(Ref<CubeMap> skybox, uint32_t size)
    {
        auto shader = Shader::Create("assets/shader/IrradianceConvolution.glsl");

        Ref<CubeMap> irradianceMap = CubeMap::Create(size);

        skybox->Bind(0);
        shader->Bind();
        shader->SetInt("u_EnvironmentMap", 0);
        
        glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        RenderUtil::Get()->RenderToCubemapFaces(irradianceMap, size, shader, proj,
            [&](int face, const glm::mat4& view)
            {
                shader->SetMat4("u_View", view);
            });

        return irradianceMap;
    }

    Ref<CubeMap> IBLUtils::CreatePrefilteredMap(Ref<CubeMap> skybox, uint32_t size)
    {
        auto shader = Shader::Create("assets/shader/PrefilterConvolution.glsl");

        Ref<CubeMap> prefilteredmap = CubeMap::Create(size);
        prefilteredmap->GenerateMipmap();

        skybox->Bind(0);
        shader->Bind();
        shader->SetInt("u_EnvironmentMap", 0);

        glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        for (uint32_t mip = 0; mip < 5; ++mip)
        {
            float roughness = (float)mip / 4.0f;
            shader->SetFloat("u_Roughness", roughness);

            RenderUtil::Get()->RenderToCubemapFaces(prefilteredmap, size, shader, proj,
                [&](int face, const glm::mat4& view)
                {
                    shader->SetMat4("u_View", view);
                }, mip);
        }

        return prefilteredmap;
    }

    Ref<Texture2D> IBLUtils::CreateBRDFLUT(uint32_t size)
    {
        auto texture = Texture2D::Create(size, size, TextureFormat::RG16F);

        auto shader = Shader::Create("assets/shader/BRDFLUT.glsl");

        RenderUtil::Get()->RenderToTexture2D(texture, size, shader, [] {});

        return texture;
    }

}
