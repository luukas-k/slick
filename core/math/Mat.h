#pragma once

#include "Core.h"

#include "Vec.h"

namespace Slick::Math {

	struct fMat4 {
		fVec4 data[4];

		inline fVec4& operator[](int i) { return data[i]; }
		inline const fVec4& operator[](int i) const { return data[i]; }
	};

	inline fMat4 perspective(float ar, float fov, float znear, float zfar) {
		float S = 1.f / tanf(fov / 2.f);
		return fMat4{
			fVec4{ S / ar,	0.f,	0.f,							0.f},
			fVec4{ 0.f,		S,		0.f,							0.f},
			fVec4{ 0.f,		0.f,	-(zfar + znear) / (zfar - znear), -1.f},
			fVec4{ 0.f,		0.f,	-1.f * (zfar * znear) / (zfar - znear), 0.f }
		};
	}

	inline fMat4 translation(fVec3 tx) {
		return fMat4{
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			tx.x, tx.y, tx.z, 1.f
		};
	}

	inline fMat4 scale(fVec3 sc) {
		return fMat4{
			sc.x, 0.f, 0.f, 0.f,
			0.f, sc.y, 0.f, 0.f,
			0.f, 0.f, sc.z, 0.f,
			0.f, 0.f, 0.f, 1.f
		};
	}

	inline fMat4 rotation_x(float ang) {
		return fMat4{
			fVec4{1.f, 0.f, 0.f, 0.f},
			fVec4{0.f, cosf(ang), sinf(ang), 0.f},
			fVec4{0.f, -sinf(ang), cosf(ang), 0.f},
			fVec4{0.f, 0.f, 0.f, 1.f }
		};
	}

	inline fMat4 rotation_y(float ang) {
		return fMat4{
			fVec4{ cosf(ang),	0.f,	sinf(ang),	0.f },
			fVec4{ 0.f,			1.f,	0.f,		0.f },
			fVec4{ -sinf(ang),	0.f,	cosf(ang),	0.f },
			fVec4{ 0.f,			0.f,	0.f,		1.f },
		};
	}

	inline fMat4 rotation_z(float ang) {
		return fMat4{
			fVec4{ cosf(ang), -sinf(ang), 0.f, 0.f },
			fVec4{ sinf(ang), cosf(ang), 0.f, 0.f }, 
			fVec4{ 0.f, 0.f, 1.f, 0.f },
			fVec4{ 0.f, 0.f, 0.f, 1.f }
		};
	}

	inline fMat4 operator*(const fMat4& lhs, const fMat4& rhs) {
		fMat4 res{};
		for (u32 i = 0; i < 4; i++) {
			for (u32 j = 0; j < 4; j++) {
				float S = 0.f;
				for (u32 k = 0; k < 4; k++) {
					S += lhs[i][k] * rhs[k][j];
				}
				res[i][j] = S;
			}
		}
		return res;
	}

	inline float radians(float deg) {
		return deg * 0.0174532925f;
	}

}
