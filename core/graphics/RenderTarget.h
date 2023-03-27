#pragma once

#include "Core.h"

#include <vector>

namespace Slick::Gfx {
	
	enum struct TextureFormat {
		RGBA,
		U32,
		Depth,
	};

	struct RTTextureSpec {
		TextureFormat fmt;
	};

	class RenderTarget {
	public:
		RenderTarget(u32 w, u32 h, const std::vector<RTTextureSpec>& textures);
		~RenderTarget();

		void bind();
		void unbind();

		inline u32 id(){ return mFramebuffer; }
		u32 get_attachment(u32 index);

		u32 read_from_buffer_u32(u32 index, i32 x, i32 y);
	private:
		u32 mFramebuffer;
		std::vector<u32> mAttachments;
	};

}