#pragma once

#include "Core.h"

#include "math/Vec.h"
#include "Shader.h"
#include "app/Scene.h"
#include "RenderTarget.h"

namespace Slick::Gfx {

	class DebugRenderer {
	public:
		DebugRenderer();
		~DebugRenderer();

		void submit_quad(Math::fVec3 pos, Math::fVec3 axis0, Math::fVec3 axis1, Math::fVec3 color);

		void update(App::Scene& scene, ECS::Manager& mgr, float dt);

		u32 current_id(i32 x, i32 y);
	private:
		Shader mShader;
		RenderTarget mRenderTarget;
		struct DebugVertex {
			Math::fVec3 pos, color;
		};
		std::vector<DebugVertex> mVertices;
	};

}