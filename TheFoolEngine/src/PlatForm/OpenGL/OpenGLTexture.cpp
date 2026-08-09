#include "tfpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

//------------------ 2D ---------------------------- //
namespace TheFoolEngine 
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width),m_Height(height), m_Data(nullptr), m_Path("")
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

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, GLenum format)
        : m_Width(width), m_Height(height), m_InternalFormat(format),
            m_DataFormat(0), m_Data(nullptr), m_Path("")
    {
        TF_PROFILE_FUNCTION();

        m_DataFormat = (format == TextureFormat::RG16F) ? GL_RG : GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, width, height);
        
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format)
        : OpenGLTexture2D(width, height, RGBTypeTranslate(format))
    {
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, AttachmentType format)
        : OpenGLTexture2D(width, height, AttachmentTypeTranslate(format))
    {
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)
        : m_Path(path.u8string())
    {
        TF_PROFILE_FUNCTION();

        uint32_t whitePixel = 0xFFFFFFFF;
        stbi_uc* data = nullptr;
        int width, height, channels;

        std::wstring wpath = path.wstring();

        FILE* fp = _wfopen(wpath.c_str(), L"rb");
        if (!fp)
        {
            TF_CORE_ERROR("Failed to load texture from path: {0}", path.u8string());
            return;
        }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::vector<stbi_uc> buffer(size);

        fread(buffer.data(), 1, size, fp);
        fclose(fp);
        stbi_set_flip_vertically_on_load(1);
        {
            TF_PROFILE_SCOPE("OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)");
            width = 1;
            height = 1;
            channels = 4;
            data = stbi_load_from_memory(buffer.data(), (int)size, &width, &height, &channels, 0);
        }

        if (!data)
        {
            TF_CORE_ERROR("Failed to load texture: {0}. Falling back to 1x1 white.", path.string());
            width = 1;
            height = 1;
            channels = 4;
            data = (stbi_uc*)&whitePixel;
        }

        m_Width = width;
        m_Height = height;

        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (channels == 1)
        {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        }
        else
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }

        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (channels == 1)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

        m_Data = nullptr;
        if (data != (stbi_uc*)&whitePixel)
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

    void OpenGLTexture2D::SetInternalFormat(TextureFormat type)
    {
        m_InternalFormat = RGBTypeTranslate(type);
    }

    void OpenGLTexture2D::SetDataFormat(TextureFormat type)
    {
        m_DataFormat = RGBTypeTranslate(type);
    }

    GLenum OpenGLTexture2D::RGBTypeTranslate(TextureFormat type)
    {
        GLenum res = NULL;

        switch (type)
        {
            case TextureFormat::RG16F:    res = GL_RG16F;     break;
            case TextureFormat::RGB:      res = GL_RGB;       break;
            case TextureFormat::RGBA:     res = GL_RGBA;      break;
            case TextureFormat::RGB8:     res = GL_RGB8;      break;
            case TextureFormat::RGBA8:    res = GL_RGBA8;     break;
            case TextureFormat::RGB16:    res = GL_RGB16;     break;
            case TextureFormat::RGBA16:   res = GL_RGBA16;    break;
            case TextureFormat::RGB16F:   res = GL_RGB16F;    break;
            case TextureFormat::RGBA16F:  res = GL_RGBA16F;   break;
            case TextureFormat::RGB32F:   res = GL_RGB32F;    break;
            case TextureFormat::RGBA32F:  res = GL_RGBA32F;   break;
        }

        return res;
    }

    GLenum OpenGLTexture2D::AttachmentTypeTranslate(AttachmentType type)
    {
        GLenum res = NULL;
        switch (type)
        {
            case TheFoolEngine::Depth:          res = GL_DEPTH_COMPONENT32F;               break;
            case TheFoolEngine::DepthStencil:   res = GL_DEPTH24_STENCIL8; break;
        }
        return res;
    }

    // === TextureCache ================================================================
    std::unordered_map<std::filesystem::path, Ref<Texture2D>> TextureCache::s_Cache;
    bool TextureCache::Regist(const std::filesystem::path& path, Ref<Texture2D> texture)
    {
        if (s_Cache.find(path) != s_Cache.end())
            return false;

        s_Cache[path] = texture;
        return true;
    }

    Ref<Texture2D> TextureCache::FindTexture(const std::filesystem::path& path)
    {
        if (s_Cache.find(path) != s_Cache.end())
            return s_Cache[path];

        return nullptr;
    }

    void TextureCache::Clear()
    {
        s_Cache.clear();
    }

}
