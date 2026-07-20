#pragma once

#include "TheFoolEngine/Core/Base.h"
#include "Texture.h"

namespace TheFoolEngine
{


	struct FrameBufferSpecification
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

        TextureFormat FrameBufferFormat = RGBA;

		bool SwapChainTarget = false;
	};

	class FrameBuffer {
	public:

		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

        virtual uint32_t GetRenderID() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);

        virtual Ref<Texture2D> GetColorAttachment() = 0;
	};
}
