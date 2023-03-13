#include "Renderer2D.h"

#include "glad/glad.h"

#include "graphics/Shader.h"

namespace Slick::Gfx {

	Renderer2D::Renderer2D() 
		:
		mScreen({0, 0, 0, 0}),
		mVertexCount(0),
		mVertices({}),
		mShader("shader/vs2d.glsl", "shader/fs2d.glsl"),
		mVertexBuffer(0),
		mCurrentVertexBufferSize(0),
		mActiveTextureCount(0)
	{
		glGenBuffers(1, &mVertexBuffer);
	}

	Renderer2D::~Renderer2D() {}

	void Renderer2D::begin() {
		mVertexCount = 0;
		mActiveTextureCount = 0;
	}

	void Renderer2D::end() {
		mShader.bind(); 

		u32 vao{};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Draw
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		if (mCurrentVertexBufferSize != mVertices.size()) {
			mCurrentVertexBufferSize = mVertices.size();
			glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex2D), mVertices.data(), GL_STATIC_DRAW);
		}
		else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, mVertexCount * sizeof(Vertex2D), mVertices.data());
		}

		for (u32 i = 0; i < mActiveTextureCount; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, mCurrentTextures[i]);
			std::string name = "sys_textures[" + std::to_string(i) + "]";
			glUniform1i(glGetUniformLocation(mShader.id(), name.c_str()), i);
		}

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, uv));
		
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, color));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, texture_index));
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
		
		glDeleteVertexArrays(1, &vao);
	}

	void Renderer2D::submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, u32 tex) {
		if (mActiveTextureCount == 16) {
			end();
			begin();
		}
		
		auto push_vertex = [&](Vertex2D v) {
			if (mVertexCount < mVertices.size()) {
				mVertices[mVertexCount++] = v;
			}
			else {
				mVertexCount++;
				mVertices.push_back(v);
			}
		};
		
		float tex_i = -1.f;
		for (u32 i = 0; i < mActiveTextureCount; i++) {
			if (mCurrentTextures[i] == tex) {
				tex_i = (float)i;
			}
		}
		if (tex_i == -1.f) {
			tex_i = (float)mActiveTextureCount;
			mCurrentTextures[mActiveTextureCount++] = tex;
		}

		p0 = p0 * 2.f - Math::fVec2{1.f, 1.f};
		p1 = p1 * 2.f - Math::fVec2{1.f, 1.f};

		push_vertex({ {p0.x, p0.y}, {uv0.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i });
		push_vertex({ {p1.x, p1.y}, {uv1.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i });
		push_vertex({ {p0.x, p1.y}, {uv0.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i });

		push_vertex({ {p1.x, p1.y}, {uv1.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i });
		push_vertex({ {p0.x, p0.y}, {uv0.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i });
		push_vertex({ {p1.x, p0.y}, {uv1.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i });
	}

	void Renderer2D::submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec3 color) {
		auto push_vertex = [&](Vertex2D v) {
			if (mVertexCount < mVertices.size()) {
				mVertices[mVertexCount++] = v;
			}
			else {
				mVertexCount++;
				mVertices.push_back(v);
			}
		};

		p0 = p0 * 2.f - Math::fVec2{1.f, 1.f};
		p1 = p1 * 2.f - Math::fVec2{1.f, 1.f};

		push_vertex({ {p0.x, p0.y}, {0.f, 0.f}, color, -1.f });
		push_vertex({ {p1.x, p1.y}, {1.f, 1.f}, color, -1.f });
		push_vertex({ {p0.x, p1.y}, {0.f, 1.f}, color, -1.f });

		push_vertex({ {p1.x, p1.y}, {1.f, 1.f}, color, -1.f });
		push_vertex({ {p0.x, p0.y}, {0.f, 0.f}, color, -1.f });
		push_vertex({ {p1.x, p0.y}, {1.f, 0.f}, color, -1.f });
	}

}
