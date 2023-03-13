#pragma once

#include "Core.h"

#include "Viewport.h"
#include "math/Vec.h"

namespace Slick::Gfx {

	class Renderer2D {
	public:
		Renderer2D();
		~Renderer2D();

		void draw_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, Math::fVec3 color);
		void draw_rect_textured(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, u32 tex);

		inline void on_resize(Viewport vp) { mScreen = vp; }
	private:
		Viewport mScreen;
	};

}