#pragma once

#include "Core.h"

#include "math/Vec.h"
#include "math/Quat.h"

namespace Slick {

	struct TransformComponent {
		Math::fVec3 position;
		Math::fVec3 scale;
		Math::fQuat rotation;
	};

}