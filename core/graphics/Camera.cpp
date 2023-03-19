#include "Camera.h"

namespace Slick::Gfx {

	Camera::Camera()
		:
		mFov(Math::radians(80.f)),
		mPosition({0.f, 0.f, 0.f}),
		mRotation({0.f, 0.f, 0.f}),
		mAspectRatio(1.f)
	{}

	Camera::~Camera() {}

	Math::fMat4 Camera::projection() {
		return Math::perspective(mAspectRatio, mFov, 0.01f, 1000.f);
	}

	Math::fMat4 Camera::view() {
		return
			Math::translation(-mPosition) *
			Math::rotation_z(-mRotation.z) *
			Math::rotation_y(-mRotation.y) *
			Math::rotation_x(-mRotation.x);
	}

	Camera& Camera::translate_global(Math::fVec3 tx) {
		mPosition = mPosition + tx;
		return *this;
	}

	Camera& Camera::translate_local(Math::fVec3 tx) {
		float sy = sinf(mRotation.y), cy = cosf(mRotation.y);
		return translate_global({
			tx.x * cy - tx.z * sy,
			tx.y,
			tx.z * cy + tx.x * sy
								});
	}

	Camera& Camera::rotate(Math::fVec3 rot) {
		mRotation = mRotation + rot;
		if (mRotation.x < -3.1415f / 2.f)
			mRotation.x = -3.1415f / 2.f;
		if (mRotation.x >  3.1415f / 2.f)
			mRotation.x = 3.1415f / 2.f;
		return *this;
	}

	void Camera::set_position(Math::fVec3 pos) {
		mPosition = pos;
	}

	void Camera::set_rotation(Math::fVec3 rot) {
		mRotation = rot;
	}

}