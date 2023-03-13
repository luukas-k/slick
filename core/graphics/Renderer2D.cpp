#include "Renderer2D.h"

#include "glad/glad.h"

#include "graphics/Shader.h"

namespace Slick::Gfx {

	struct vertex {
		Math::fVec2 pos;
		Math::fVec2 uv;
	};

	Renderer2D::Renderer2D() 
		:
		mScreen({0, 0, 0, 0})
	{}

	Renderer2D::~Renderer2D() {

	}

	void Renderer2D::draw_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, Math::fVec3 color) {
		p0 = p0 * 2.f - Math::fVec2{1.f, 1.f};
		p1 = p1 * 2.f - Math::fVec2{1.f, 1.f};

		Shader shader("shader/vs2d.glsl", "shader/fs2d.glsl");
		shader.bind();

		glUniform3f(glGetUniformLocation(shader.id(), "sys_color"), color.x, color.y, color.z);
		
		u32 vao{}, vbo{};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		std::vector<vertex> data{
			{ {p0.x, p0.y}, {uv0.x, uv0.y} },
			{ {p1.x, p1.y}, {uv1.x, uv1.y} },
			{ {p0.x, p1.y}, {uv0.x, uv1.y} },

			{ {p1.x, p1.y}, {uv1.x, uv1.y} },
			{ {p0.x, p0.y}, {uv0.x, uv0.y} },
			{ {p1.x, p0.y}, {uv1.x, uv0.y} },
		};

		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(vertex), data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(vertex), (const void*)offsetof(vertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(vertex), (const void*)offsetof(vertex, uv));

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)data.size());
		
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	void Renderer2D::draw_rect_textured(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, u32 texture) {
		Shader shader("shader/vs2d.glsl", "shader/vs2dtex.glsl");

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(shader.id(), "sys_texture"), 0);
		
		u32 vao{}, vbo{};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		std::vector<vertex> data{
			{ {p0.x, p0.y}, {uv0.x, uv0.y} },
			{ {p0.x, p1.y}, {uv0.x, uv1.y} },
			{ {p1.x, p1.y}, {uv1.x, uv1.y} },

			{ {p1.x, p1.y}, {uv1.x, uv1.y} },
			{ {p1.x, p0.y}, {uv1.x, uv0.y} },
			{ {p0.x, p0.y}, {uv0.x, uv0.y} }
		};

		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(vertex), data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(vertex), (const void*)offsetof(vertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(vertex), (const void*)offsetof(vertex, uv));

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)data.size());
		
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

}
