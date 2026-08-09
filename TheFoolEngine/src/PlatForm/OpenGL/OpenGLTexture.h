#pragma once

#include "TheFoolEngine/Renderer/Texture.h"

#include <glad/glad.h>

//------------------ 2D ---------------------------- //
namespace TheFoolEngine 
{

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(uint32_t width, uint32_t height, GLenum format);
		OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format);
		OpenGLTexture2D(uint32_t width, uint32_t height, AttachmentType format);
		OpenGLTexture2D(const std::filesystem::path& path);
		OpenGLTexture2D(const void* data, uint32_t size);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetSubTextureData(void* data, int xoffset, int yoffset, int width, int height, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override 
		{ 
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID; 
		}

		virtual GLenum GetInternalFormat() const override;
		virtual GLenum GetDataFormat() const override;

		virtual void* GetData() const override;

		virtual std::string GetPath() const override { return m_Path; }

        void SetInternalFormat(TextureFormat type);
        void SetDataFormat(TextureFormat type);

    public:
        static GLenum RGBTypeTranslate(TextureFormat type);
        static GLenum AttachmentTypeTranslate(AttachmentType type);
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		void* m_Data;
	};

    class TextureCache
    {
    public:
        static bool Regist(const std::filesystem::path& path, Ref<Texture2D> texture);
        static Ref<Texture2D> FindTexture(const std::filesystem::path& path);
        static void Clear();
    private:
        TextureCache() = delete;
    private:
        static std::unordered_map<std::filesystem::path, Ref<Texture2D>> s_Cache;
    };
}



