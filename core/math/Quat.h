#pragma once

#include "Core.h"

#include "utility/Logger.h"
#include "Vec.h"
#include "Mat.h"

namespace Slick::Math {

	struct fQuat {
		float x, y, z, w;
	};

	inline float length(const fQuat& v) {
		return sqrt(v.x * v.y + v.y * v.y + v.z * v.z + v.w * v.w);
	}

	inline fQuat operator*(const fQuat& v, float r) {
		return fQuat{
			v.x * r,
			v.y * r,
			v.z * r,
			v.w * r
		};
	}

	inline fQuat operator/(const fQuat& v, float r) {
		return fQuat{
			v.x / r,
			v.y / r,
			v.z / r,
			v.w / r
		};
	}

	inline fQuat normalize(const fQuat& v) {
		return v / (length(v) + 0.0001f);
	}

	inline fQuat axis_rotation(fVec3 axis, float angle) {
		float sHalfA = sin(angle / 2.f);
		return normalize(fQuat{
			sHalfA * axis.x,
			sHalfA * axis.y,
			sHalfA * axis.z,
			cos(angle / 2.f)
		});
	}

	inline fQuat operator*(const fQuat& lhs, const fQuat& rhs) {
		float a1 = lhs.x;
		float b1 = lhs.y;
		float c1 = lhs.z;
		float d1 = lhs.w;
		float a2 = rhs.x;
		float b2 = rhs.y;
		float c2 = rhs.z;
		float d2 = rhs.w;
		return fQuat{
			a1 * d2 + b1 * c2 + c1 * b2 + d1 * a2,
			a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2,
			a1 * b2 + b1 * a2 + c1 * d2 - d1 * c2,
			a1 * c2 - b1 * d2 + c1 * a2 + d1 * b2
		};
	}

	inline fMat4 rotation_matrix(fQuat rot) {
		float q0 = rot.x;
		float q1 = rot.x;
		float q2 = rot.x;
		float q3 = rot.x;
		return fMat4{
			fVec4{1.f - 2.f * (q0 * q0 + q1 * q1), 2.f * (q1 * q2 - q0 * q3), 2.f * (q1 * q3 + q0 * q2), 0.f},
			fVec4{2.f * (q1 * q2 + q0 * q3), 1.f - 2.f * (q0 * q0 + q2 * q2), 2.f * (q2 * q3 - q0 * q1), 0.f},
			fVec4{2.f * (q1 * q3 - q0 * q2), 2.f * (q2 * q3 + q0 * q1), 1.f - 2.f * (q0 * q0 + q3 * q3), 0.f},
			fVec4{0.f, 0.f, 0.f, 1.f},
		};
	}

}


namespace Slick {

	template<>
	inline std::string format<Math::fQuat>(Math::fQuat v) {
		return "vec3{" + format(v.x) + " " + format(v.y) + " " + format(v.z) + " " + format(v.w) + "}";
	}

}