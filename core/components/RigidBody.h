#pragma once

#include "Core.h"

namespace Slick {

	struct RigidBody {
		Math::fVec3 velocity, angularVelocity;
	};

	struct SphereCollider {
		float radius;
	};

}