#include "RenderTarget.h"

#include <glad/glad.h>

#include "utility/Logger.h"

namespace Slick::Gfx {
	
	RenderTarget::RenderTarget(u32 w, u32 h, const std::vector<RTTextureSpec>& textures) 
		:
		mFramebuffer(0), mAttachments({})
	{
		glGenFramebuffers(1, &mFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

		for (auto& t : textures) {
			u32 tex{};
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			if (t.fmt == TextureFormat::RGBA) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + mAttachments.size(), GL_TEXTURE_2D, tex, 0);
			}
			else if (t.fmt == TextureFormat::Depth) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
			}

			mAttachments.push_back(tex);
		}

		u32 stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (stat != GL_FRAMEBUFFER_COMPLETE) {
			Utility::Assert(false);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	RenderTarget::~RenderTarget() {
		for (auto& attach : mAttachments) {
			glDeleteTextures(1, &attach);
		}
		glDeleteFramebuffers(1, &mFramebuffer);
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

}
