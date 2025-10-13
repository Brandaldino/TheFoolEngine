#include "tfpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace TheFoolEngine {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
		switch (type)
		{
			case TheFoolEngine::ShaderDataType::None:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Float:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Float2:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Float3:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Float4:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Mat3:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Mat4:		return GL_FLOAT;
			case TheFoolEngine::ShaderDataType::Int:		return GL_INT;
			case TheFoolEngine::ShaderDataType::Int2:		return GL_INT;
			case TheFoolEngine::ShaderDataType::Int3:		return GL_INT;
			case TheFoolEngine::ShaderDataType::Int4:		return GL_INT;
			case TheFoolEngine::ShaderDataType::Bool:		return GL_BOOL;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray() {
		TF_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {
		TF_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const {
		TF_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const {
		TF_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		TF_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		TF_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		TF_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}
