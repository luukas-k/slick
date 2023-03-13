#include "Font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "glad/glad.h"

namespace Slick::Gfx {

	std::vector<u8> load_file(const std::string& fname) {
		std::fstream fs(fname, std::fstream::in | std::fstream::binary);
		if(!fs) 
			return {};
		return std::vector<u8>(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
	}

	UIFont::UIFont(const std::string& fname) {
		auto data = load_file(fname);
		u8* image = new u8[512 * 512];
		std::vector<stbtt_bakedchar> cd(96, stbtt_bakedchar{});
		stbtt_BakeFontBitmap(data.data(), 0, 32.f, image, 512, 512, 32, 96, cd.data());

		u32 tex{};
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		mData = data,
		mCharDesc = cd;
		mTexture = tex;
		mScale = 32.f;
	}

	UIFont::~UIFont() {}

	std::pair<i32, i32> UIFont::text_metrics(Viewport vp, const std::string& text) {
		auto w = vp.w;
		auto h = vp.h;
		float xp = 0.f, yp = 0.f;
		float mdy = 0.f;
		for (char c : text) {
			stbtt_aligned_quad aq{};
			stbtt_GetBakedQuad(chardata(), 512, 512, c - 32, &xp, &yp, &aq, 1);
			float scale_x = 1.f / w;
			float scale_y = 1.f / h;
			float h0 = aq.y1 - aq.y0;
			Math::fVec2 
				p0{aq.x0 * scale_x, (aq.y1 + h0) * scale_y}, 
				p1{aq.x1 * scale_x, (aq.y1) * scale_y};
			float dy = abs(p0.y - p1.y);
			if(dy > mdy)
				mdy = dy;
		}
		return {xp, mdy * h};
	}

}

