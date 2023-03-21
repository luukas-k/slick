#pragma once

#include "Core.h"

#include "app/Scene.h"
#include "ecs/Manager.h"

namespace Slick::Physics {

	class PhysicsSystem {
	public:
		PhysicsSystem();
		~PhysicsSystem();

		void fixed_update(App::Scene& scene, ECS::Manager& mgr, float dt);
	};

}