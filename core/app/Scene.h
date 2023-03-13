#pragma once

#include "Core.h"

#include "ecs/Manager.h"

#include "graphics/Camera.h"

namespace Slick::App {

	class Scene {
	public:
		Scene();
		~Scene();

		void update(float dt);

		inline ECS::Manager& manager() { return mManager; }
		inline Gfx::Camera& camera() { return mCamera; }
	private:
		ECS::Manager mManager;
		Gfx::Camera mCamera;
	};

}
