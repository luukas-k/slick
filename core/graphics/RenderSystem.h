#pragma once

#include "Core.h"

#include "app/Scene.h"
#include "graphics/Camera.h"
#include "app/ResourceManager.h"
#include "ecs/Manager.h"
#include "graphics/Shader.h"
#include "components/Transform.h"
#include "components/Renderable.h"
#include "components/Light.h"

namespace Slick::Gfx {

	class RenderSystem {
	public:
		RenderSystem();
		~RenderSystem();

		void update(App::Scene& scene, ECS::Manager& mgr, float dt);
	private:
		u32 vao{};
		Gfx::Shader mProgram;
	};

}