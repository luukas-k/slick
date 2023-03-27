#pragma once

#include "Core.h"

#include "app/Scene.h"
#include "graphics/RenderTarget.h"
#include "graphics/Camera.h"
#include "app/ResourceManager.h"
#include "ecs/Manager.h"
#include "graphics/Shader.h"
#include "components/Transform.h"
#include "components/Renderable.h"
#include "components/Light.h"
#include "Viewport.h"

namespace Slick::Gfx {

	class RenderSystem {
	public:
		RenderSystem();
		~RenderSystem();

		void update(App::Scene& scene, ECS::Manager& mgr, float dt);
		inline void set_viewport(Viewport vp) { mScreen = vp; }
	private:
		u32 vao{}, mSkybox;
		Viewport mScreen;
		Shader mProgram, mSkyShader;
		RenderTarget mRenderTarget;
	};

}