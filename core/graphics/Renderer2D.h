#pragma once

#include "Core.h"

#include "Viewport.h"
#include "math/Vec.h"

#include "Shader.h"

namespace Slick::Gfx {

	struct Vertex2D {
		Math::fVec2 pos;
		Math::fVec2 uv;
		Math::fVec3 color;
		float texture_index;
		float quad_ar;
		float border_radius;
	};

	class Renderer2D {
	public:
		Renderer2D();
		~Renderer2D();

		void begin();
		void end();
		
		void submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec3 color, float border_radius);
		void submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, u32 texture, float border_radius);

		inline void on_resize(Viewport vp) { mScreen = vp; }
	private:
		Viewport mScreen;
		u64 mVertexCount;
		std::vector<Vertex2D> mVertices;
		Shader mShader;
		u32 mVertexBuffer, mCurrentVertexBufferSize;

		std::array<u32, 16> mCurrentTextures;
		u32 mActiveTextureCount;
	};

}