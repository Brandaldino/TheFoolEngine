#pragma once

#include <string>

#include "TheFoolEngine/Core/Base.h"

namespace TheFoolEngine 
{

    enum TextureFormat : uint8_t
    {
        RG16F,
        RGB,
        RGBA,
        RGB8,
        RGBA8,
        RGB16,
        RGBA16,
        RGB16F,
        RGBA16F,
        RGB32F,
        RGBA32F
    };

    struct TextureSpec
    {
        bool IsDefaultSetting = true;

        uint32_t Width = 1;
        uint32_t Height = 1;
        

    };

	class Texture 
    {
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetSubTextureData(void* data, int xoffset, int yoffset, int width, int height, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;

		virtual unsigned int GetInternalFormat() const = 0;
		virtual unsigned int GetDataFormat() const = 0;
		virtual void* GetData() const = 0;

		virtual std::string GetPath() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() = default;

		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
        static Ref<Texture2D> Create(uint32_t width, uint32_t height, TextureFormat format);
        static Ref<Texture2D> Create(const std::filesystem::path& path);
		static Ref<Texture2D> Create(const void* data, uint32_t size);
	};

}
