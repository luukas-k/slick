#pragma once

#include "Core.h"

#include "utility/Logger.h"
#include "math/Vec.h"
#include "Shader.h"
#include "app/Scene.h"
#include "RenderTarget.h"

namespace Slick::Gfx {

	enum struct SelectedAxis {
		None,
		X, Y, Z,
		XY, XZ, YZ
	};

	class DebugRenderer {
	public:
		DebugRenderer();
		~DebugRenderer();

		inline void resize(u32 w, u32 h) { mRenderTarget.resize(w, h); }

		void submit_quad(Math::fVec3 pos, Math::fVec3 axis0, Math::fVec3 axis1, Math::fVec3 color, u32 id);
		SelectedAxis submit_translate_gizmo(Math::fVec3 pos, i32 x, i32 y);
		void draw_grid(i32 w, i32 d);

		void update(App::Scene& scene, ECS::Manager& mgr, float dt);

		u32 current_id(i32 x, i32 y);
	private:
		Shader mShader;
		RenderTarget mRenderTarget;
		struct DebugVertex {
			Math::fVec3 pos, color;
			u32 id;
		};
		std::vector<DebugVertex> mVertices;
	};

}

namespace Slick {

	template<>
	inline std::string format<Gfx::SelectedAxis>(Gfx::SelectedAxis axis) {
		switch (axis) {
			case Gfx::SelectedAxis::X: return "X";
			case Gfx::SelectedAxis::Y: return "Y";
			case Gfx::SelectedAxis::Z: return "Z";
		}
		return "None";
	}

}