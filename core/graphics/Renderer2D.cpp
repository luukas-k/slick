#include "Renderer2D.h"

#include "glad/glad.h"

#include "graphics/Shader.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace Slick::Gfx {

	Renderer2D::Renderer2D() 
		:
		mScreen({0, 0, 0, 0}),
		mVertexCount(0),
		mVertices({}),
		mShader("shader/vs2d.glsl", "shader/fs2d.glsl"),
		mVertexBuffer(0),
		mCurrentVertexBufferSize(0),
		mActiveTextureCount(0),
		mCurrentTextures({})
	{
		glGenBuffers(1, &mVertexBuffer);

		auto read_file = [](const std::string& fname) -> std::vector<u8> {
			std::fstream fs(fname, std::fstream::binary | std::fstream::in);
			if(!fs) return {};
			return std::vector<u8>(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
		};

		auto font_data = read_file("font/Roboto-Regular.ttf");
		// std::vector<stbtt_packedchar> chardata;
		mFontData.resize(95);
		
		u8* font_bitmap = new u8[512 * 512];
		
		stbtt_pack_context pctx{};

		stbtt_PackBegin(&pctx, font_bitmap, 512, 512, 0, 1, nullptr);
		stbtt_PackFontRange(&pctx, font_data.data(), 0, 32.f, 32, 95, mFontData.data());
		stbtt_PackEnd(&pctx);


		u32 ftex{};
		glGenTextures(1, &ftex);
		glBindTexture(GL_TEXTURE_2D, ftex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, font_bitmap);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		delete[] font_bitmap;

		mFontTexture = ftex;
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

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

		// Draw
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		if (mCurrentVertexBufferSize != mVertices.size()) {
			mCurrentVertexBufferSize = (u32)mVertices.size();
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

		mShader.set_uniform_f2("sys_viewport", {(float)mScreen.w, (float)mScreen.h});

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, uv));
		
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, color));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, texture_index));
		
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, quad_size));
		
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 1, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, border_radius));
		
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 1, GL_FLOAT, false, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, is_text));
		
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mVertexCount);
		
		glDeleteVertexArrays(1, &vao);
	}

	void Renderer2D::submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec2 uv0, Math::fVec2 uv1, u32 tex, float b_radius, bool is_text) {
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

		float quad_ar = (p1.x * mScreen.w - p0.x * mScreen.w) / (p1.y * mScreen.h - p0.y * mScreen.h);

		Math::fVec2 size{
			(p1.x - p0.x) * mScreen.w,
			(p1.y - p0.y) * mScreen.h
		};

		p0 = p0 * 2.f - Math::fVec2{1.f, 1.f};
		p1 = p1 * 2.f - Math::fVec2{1.f, 1.f};

		// Utility::Log("ar: ", quad_ar);

		push_vertex({ {p0.x, p0.y}, {uv0.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });
		push_vertex({ {p1.x, p1.y}, {uv1.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });
		push_vertex({ {p0.x, p1.y}, {uv0.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });

		push_vertex({ {p1.x, p1.y}, {uv1.x, uv1.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });
		push_vertex({ {p0.x, p0.y}, {uv0.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });
		push_vertex({ {p1.x, p0.y}, {uv1.x, uv0.y}, {0.f, 0.f, 0.f}, tex_i, size, b_radius, is_text ? 1.f : 0.f });
	}

	void Renderer2D::submit_text(Math::fVec2 pos, float scale1, const std::string& text) {
		for (char c : text) {
			const stbtt_packedchar& pc = mFontData[c - 32];
			float scale = scale1 / 32.f;
			float ar = (float)mScreen.h / mScreen.w;
			Math::fVec2 p0{
				pc.xoff * scale * ar,
				-pc.yoff2 * scale
			};
			Math::fVec2 p1{
				pc.xoff2 * scale * ar,
				-pc.yoff * scale
			};
			submit_rect(pos + p0, pos + p1, {(float)pc.x0 / 512, (float)pc.y1 / 512}, {(float)pc.x1 / 512, (float)pc.y0 / 512}, mFontTexture, 0.f, true);
			pos.x += pc.xadvance * scale * ar;
		}
	}

	Math::fVec2 Renderer2D::text_metrics(float scale1, const std::string& text) {
		Math::fVec2 pos{0.f, scale1};
		for (char c : text) {
			const stbtt_packedchar& pc = mFontData[c - 32];
			float scale = scale1 / 32.f;
			float ar = (float)mScreen.h / mScreen.w;
			Math::fVec2 p0{
				pc.xoff * scale * ar,
				-pc.yoff2 * scale
			};
			Math::fVec2 p1{
				pc.xoff2 * scale * ar,
				-pc.yoff * scale
			};
			pos.x += pc.xadvance * scale * ar;
		}
		return pos;
	}

	void Renderer2D::submit_quad(Math::fVec2 pos, Math::fVec2 size, float angle, Math::fVec3 color, float border_radius) {
		auto push_vertex = [&](Vertex2D v) {
			if (mVertexCount < mVertices.size()) {
				mVertices[mVertexCount++] = v;
			}
			else {
				mVertexCount++;
				mVertices.push_back(v);
			}
		};

		/*Math::fVec2 size{
			(p1.x - p0.x) * mScreen.w,
			(p1.y - p0.y) * mScreen.h
		};*/

		Math::fVec2 p0 = (pos - size * 0.5f) * 2.f - Math::fVec2{1.f, 1.f};
		Math::fVec2 p1 = (pos + size * 0.5f) * 2.f - Math::fVec2{1.f, 1.f};

		Math::fVec2 ssize{(float)mScreen.w, (float)mScreen.h};

		push_vertex({ .pos = {p0.x, p0.y}, .uv = {0.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p1.x, p1.y}, .uv = {1.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p0.x, p1.y}, .uv = {0.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });

		push_vertex({ .pos = {p1.x, p1.y}, .uv = {1.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p0.x, p0.y}, .uv = {0.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p1.x, p0.y}, .uv = {1.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size * ssize, .border_radius = border_radius, .is_text = 0.f });
	}

	void Renderer2D::submit_rect(Math::fVec2 p0, Math::fVec2 p1, Math::fVec3 color, float border_radius) {
		Math::fVec2 qsize{
			p1.x - p0.x,
			p1.y - p0.y
		};
		submit_quad(p0 + qsize * 0.5f, qsize, 0.f, color, border_radius);
		return;

		auto push_vertex = [&](Vertex2D v) {
			if (mVertexCount < mVertices.size()) {
				mVertices[mVertexCount++] = v;
			}
			else {
				mVertexCount++;
				mVertices.push_back(v);
			}
		};

		float quad_ar = ((p1.x - p0.x) / (p1.y - p0.y)) * ((float)mScreen.w / mScreen.h);

		Math::fVec2 size{
			(p1.x - p0.x) * mScreen.w,
			(p1.y - p0.y) * mScreen.h
		};

		p0 = p0 * 2.f - Math::fVec2{1.f, 1.f};
		p1 = p1 * 2.f - Math::fVec2{1.f, 1.f};

		push_vertex({ .pos = {p0.x, p0.y}, .uv = {0.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p1.x, p1.y}, .uv = {1.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p0.x, p1.y}, .uv = {0.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });

		push_vertex({ .pos = {p1.x, p1.y}, .uv = {1.f, 1.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p0.x, p0.y}, .uv = {0.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });
		push_vertex({ .pos = {p1.x, p0.y}, .uv = {1.f, 0.f}, .color = color, .texture_index = -1.f, .quad_size = size, .border_radius = border_radius, .is_text = 0.f });
	}

}
