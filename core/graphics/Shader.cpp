#include "Shader.h"
#include "Shader.h"

#include "glad/glad.h"

#include "utility/Logger.h"

namespace Slick::Gfx {

	Shader::Shader(const std::string& vs_fname, const std::string& fs_fname) 
		:
		mProgram(0),
		mShouldUpdate(true),
		mVsFile(vs_fname), mFsFile(fs_fname)
	{
		update_program();
		mMonitor.monitor(vs_fname, [&]() {
			mShouldUpdate = true;
		});
		mMonitor.monitor(fs_fname, [&]() {
			mShouldUpdate = true;
		});
	}

	Shader::~Shader() {
		glDeleteProgram(mProgram);
	}

	void Shader::bind() {
		update_program();
		glUseProgram(mProgram);
	}

	void Shader::set_uniform_f1(const std::string& name, float v) {
		glUniform1f(uniform_location(name), v);
	}

	void Shader::set_uniform_f2(const std::string& name, const Math::fVec2 v) {
		glUniform2f(uniform_location(name), v.x, v.y);
	}

	void Shader::set_uniform_f3(const std::string& name, const Math::fVec3 v) {
		glUniform3f(uniform_location(name), v.x, v.y, v.z);
	}

	void Shader::set_uniform_m4(const std::string& name, const Math::fMat4 v) {
		glUniformMatrix4fv(uniform_location(name), 1, false, &v[0][0]);
	}

	void Shader::set_uniform_i1(const std::string& name, i32 v) {
		glUniform1i(uniform_location(name), v);
	}

	void Shader::update_program() {
		if(!mShouldUpdate) return;
		mShouldUpdate = false;
		
		u32 prog = glCreateProgram(),
			vs = glCreateShader(GL_VERTEX_SHADER), 
			fs = glCreateShader(GL_FRAGMENT_SHADER);

		auto read_file = [](const std::string& fname) -> std::string {
			std::fstream fs(fname);
			if(!fs) return "";
			return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
		};

		std::string vs_src_d = read_file(mVsFile);
		std::string fs_src_d = read_file(mFsFile);

		const char* vs_src = vs_src_d.c_str();
		const char* fs_src = fs_src_d.c_str();
		
		glShaderSource(vs, 1, &vs_src, nullptr);
		glShaderSource(fs, 1, &fs_src, nullptr);

		glCompileShader(vs);
		glCompileShader(fs);

		glAttachShader(prog, vs);
		glAttachShader(prog, fs);

		glLinkProgram(prog);

		bool success = true;

		i32 len{};
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		if (len > 0) {
			char* log = new char[len];
			glGetProgramInfoLog(prog, len, nullptr, log);
			Utility::Log("Program: ", (const char*)log);
			delete[] log;
			success = false;
		}

		glDeleteShader(vs);
		glDeleteShader(fs);

		if(success)
			mProgram = prog;
	}

	i32 Shader::uniform_location(const std::string& name) {
		return glGetUniformLocation(mProgram, name.c_str());
	}

}
