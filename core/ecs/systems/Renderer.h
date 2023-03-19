#pragma once

#include "Core.h"

#include "graphics/Camera.h"
#include "app/ResourceManager.h"
#include "ecs/Manager.h"
#include "graphics/Shader.h"
#include "ecs/components/Transform.h"
#include "ecs/components/Renderable.h"
#include "ecs/components/Light.h"

namespace Slick::Gfx {

	class RenderSystem {
	public:
		RenderSystem(Gfx::Camera& cam, App::ResourceManager& resources);
		~RenderSystem();

		void update(ECS::Manager& mgr, float dt);
	private:
		Gfx::Camera& mCamera;
		u32 vao{};
		Gfx::Shader mProgram;
		App::ResourceManager& mResources;
	};

}