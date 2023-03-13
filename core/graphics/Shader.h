#pragma once

#include "Core.h"

#include "math/Vec.h"
#include "math/Mat.h"

namespace Slick::Gfx {

	class Shader {
	public:
		Shader(const std::string& vs, const std::string& fs);
		~Shader();

		void bind();
		inline u32 id() { return mProgram; };

		void set_uniform_f1(const std::string& name, float v);
		void set_uniform_f3(const std::string& name, const Math::fVec3 color);

		void set_uniform_m4(const std::string& name, const Math::fMat4 color);
		
		void set_uniform_i1(const std::string& name, i32 v);
	private:
		i32 uniform_location(const std::string& name);

	private:
		u32 mProgram;
	};

}