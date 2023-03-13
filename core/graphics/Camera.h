#pragma once

#include "Core.h"

#include "math/Mat.h"
#include "math/Vec.h"

namespace Slick::Gfx {

	class Camera {
public:
	Camera();
	~Camera();

	Math::fMat4 projection(float ar);
	Math::fMat4 view();

	Camera& translate_global(Math::fVec3 tx);
	Camera& translate_local(Math::fVec3 tx);
	Camera& rotate(Math::fVec3 rot);

	void set_position(Math::fVec3 pos);
	void set_rotation(Math::fVec3 rot);

	inline Math::fVec3& pos() { return mPosition; }
private:
	float mFov;
	Math::fVec3 mPosition, mRotation;
};

}