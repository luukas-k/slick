#include "RenderTarget.h"

#include <glad/glad.h>

#include "utility/Logger.h"

namespace Slick::Gfx {
	
	RenderTarget::RenderTarget(u32 w, u32 h, const std::vector<RTTextureSpec>& textures) 
		:
		mFramebuffer(0), mAttachments({}),
		mWidth(0), mHeight(0), mSpec(textures)
	{
		resize(w, h);
	}
	
	RenderTarget::~RenderTarget() {
		for (auto& attach : mAttachments) {
			glDeleteTextures(1, &attach);
		}
		glDeleteFramebuffers(1, &mFramebuffer);
	}

	void RenderTarget::resize(u32 w, u32 h) {
		if (mWidth == w && mHeight == h) {
			return;
		}
		mWidth = w;
		mHeight = h;

		if (mFramebuffer) {
			glDeleteFramebuffers(1, &mFramebuffer);
			for (auto& attach : mAttachments) {
				glDeleteTextures(1, &attach);
			}
			mAttachments.clear();
		}

		glGenFramebuffers(1, &mFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

		for (auto& t : mSpec) {
			u32 tex{};
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			if (t.fmt == TextureFormat::RGBA) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + mAttachments.size(), GL_TEXTURE_2D, tex, 0);
			}
			else if (t.fmt == TextureFormat::U32) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + mAttachments.size(), GL_TEXTURE_2D, tex, 0);
			}
			else if (t.fmt == TextureFormat::Depth) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
			}
			else {
				Utility::Assert(false);
			}

			mAttachments.push_back(tex);
		}

		u32 stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (stat != GL_FRAMEBUFFER_COMPLETE) {
			Utility::Assert(false);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderTarget::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
	}

	void RenderTarget::unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	u32 RenderTarget::get_attachment(u32 index) {
		return mAttachments[index];
	}

	u32 RenderTarget::read_from_buffer_u32(u32 index, i32 x, i32 y) {
		bind();
		u32 data{};
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
		unbind();
		return data;
	}

}
