#pragma once

#include "Core.h"

#include "utility/Logger.h"

namespace Slick::Math {

	struct fVec2 {
		float x, y;
	};

	inline fVec2 operator*(const fVec2& lhs, float rhs) {
		return fVec2{
			lhs.x * rhs, 
			lhs.y * rhs, 
		};
	}

	inline fVec2 operator-(const fVec2& lhs, const fVec2& rhs) {
		return fVec2{
			lhs.x - rhs.x, 
			lhs.y - rhs.y, 
		};
	}

	inline fVec2 operator+(const fVec2& lhs, const fVec2& rhs) {
		return fVec2{
			lhs.x + rhs.x, 
			lhs.y + rhs.y, 
		};
	}

	struct fVec3 {
		float x, y, z;
	};

	inline fVec3 operator-(const fVec3& v) {
		return {-v.x, -v.y, -v.z};
	}
	
	inline fVec3 operator+(const fVec3& lhs, const fVec3& rhs) {
		return fVec3{
			lhs.x + rhs.x, 
			lhs.y + rhs.y, 
			lhs.z + rhs.z
		};
	}

	inline fVec3 operator*(const fVec3& lhs, float rhs) {
		return fVec3{
			lhs.x * rhs, 
			lhs.y * rhs, 
			lhs.z * rhs
		};
	}

	struct fVec4 {
		float x, y, z, w;

		inline float& operator[](int i) { return (&x)[i]; }
		inline const float& operator[](int i) const { return (&x)[i]; }
	};

}


namespace Slick {

	template<>
	inline std::string format<Math::fVec3>(Math::fVec3 v) {
		return "vec3{" + format(v.x) + " " + format(v.y) + " " + format(v.z) + "}";
	}

}