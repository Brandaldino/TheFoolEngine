#include "tfpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace TheFoolEngine 
{
	void OpenGLRendererAPI::Init() 
    {
		TF_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) 
    {
		glViewport(x, y, width, height);
	}


	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) 
    {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear() 
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) 
    {
		uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRendererAPI::SetDepthFunc(DepthFunc func)
	{
        glDepthFunc(func == DepthFunc::Less ? GL_LESS : GL_LEQUAL);
	}

    void OpenGLRendererAPI::SetDepthWrite(DepthWrite flag)
    {
        glDepthMask(flag == DepthWrite::On ? GL_TRUE : GL_FALSE);
    }

    void OpenGLRendererAPI::SetDepthTest(DepthTest flag)
    {
        if (flag == DepthTest::On)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetCullMode(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::None:        glDisable(GL_CULL_FACE); break;
            case CullMode::Front:       glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
            case CullMode::Back:        glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
        }
    }

    void OpenGLRendererAPI::SetPolygonMode(PolygonMode mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, mode == PolygonMode::Line ? GL_LINE : GL_FILL);
    }

    void OpenGLRendererAPI::SetBlendMode(BlendMode mode)
    {
        switch (mode)
        {
            case BlendMode::None:       glBlendFunc(GL_ONE, GL_ZERO); break;
            case BlendMode::Alpha:      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
            case BlendMode::Additive:   glBlendFunc(GL_ONE, GL_ONE); break;
        }
    }

    void OpenGLRendererAPI::SetBlend(bool flag)
    {
        if (flag)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }

    void OpenGLRendererAPI::DrawArrays(DrawMode mode, uint32_t count)
    {
        switch (mode)
        {
            case DrawMode::Triangles: glDrawArrays(GL_TRIANGLES, 0, count); break;
            default: break;
        }
    }

}
