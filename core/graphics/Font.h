#pragma once

#include "Core.h"

#include "stb_truetype.h"
#include "Viewport.h"

#include "math/Vec.h"

namespace Slick::Gfx {

	class UIFont {
	public:
		UIFont(const std::string& fname);
		~UIFont();

		inline stbtt_bakedchar* chardata() { return mCharDesc.data(); }
		inline u32 texture() { return mTexture; }

		std::pair<i32, i32> text_metrics(Viewport vp, const std::string& text);
	private:
		std::vector<u8> mData;
		std::vector<stbtt_bakedchar> mCharDesc;
		u32 mTexture;
		float mScale;
	};

}