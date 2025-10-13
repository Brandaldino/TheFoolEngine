#include "tfpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

//------------------ 2D ---------------------------- //
namespace TheFoolEngine {
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width),m_Height(height), m_Data(nullptr)
	{
		TF_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		:m_Path(path)
	{
		TF_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1); // set whether to filp or not
		stbi_uc* data = nullptr;
		{
			TF_PROFILE_SCOPE("OpenGLTexture2D::OpenGLTexture2D(const std::string& path)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		TF_CORE_ASSERT(data, "Failed to load image.");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		TF_CORE_ASSERT(internalFormat & dataFormat, "Format not supported.");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		m_Data = data;

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const void* data, uint32_t size)
		: m_Path("")
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1); // set whether to filp or not
		stbi_uc* subdata = nullptr;
		{
			subdata = stbi_load_from_memory((stbi_uc*)data, size, &width, &height, &channels, 0);
		}
		TF_CORE_ASSERT(data, "Failed to load image.");

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		TF_CORE_ASSERT(internalFormat & dataFormat, "Format not supported.");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, subdata);

		m_Data = subdata;

		stbi_image_free(subdata);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		TF_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		TF_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		TF_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture.");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

		m_Data = data;
	}

	void OpenGLTexture2D::SetSubTextureData(void* data, int xoffset, int yoffset, int width, int height, uint32_t size)
	{
		TF_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		TF_CORE_ASSERT(size == width * height * bpp, "Data must be entire texture.");

		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexSubImage2D(
			m_RendererID, 0,
			xoffset, yoffset,	// sub-texture position
			width, height,		// sub-texture size
			m_DataFormat,
			GL_UNSIGNED_BYTE,
			data
		);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		TF_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

	GLenum OpenGLTexture2D::GetInternalFormat() const
	{
		return m_InternalFormat;
	}

	GLenum OpenGLTexture2D::GetDataFormat() const
	{
		return m_DataFormat;
	}

	void* OpenGLTexture2D::GetData() const
	{
		return m_Data;
	}
}
