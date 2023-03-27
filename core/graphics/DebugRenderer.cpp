#include "DebugRenderer.h"

#include "glad/glad.h"

namespace Slick::Gfx {
	
	DebugRenderer::DebugRenderer() 
		:
		mShader("shader/vs_debug.glsl", "shader/fs_debug.glsl"),
		mRenderTarget(1920, 1080, { { TextureFormat::RGBA }, { TextureFormat::U32 }, { TextureFormat::Depth }})
	{}

	DebugRenderer::~DebugRenderer() {
	}

	void DebugRenderer::submit_quad(Math::fVec3 pos, Math::fVec3 axis0, Math::fVec3 axis1, Math::fVec3 color, u32 id) {
		std::vector<Math::fVec3> positions{
			pos + axis0 * 0.f + axis1 * 0.f,
			pos + axis0 * 0.f + axis1 * 1.f,
			pos + axis0 * 1.f + axis1 * 1.f,

			pos + axis0 * 1.f + axis1 * 1.f,
			pos + axis0 * 1.f + axis1 * 0.f,
			pos + axis0 * 0.f + axis1 * 0.f
		};

		for (auto& v : positions) {
			mVertices.push_back({ v, color, id });
		}
	}

	SelectedAxis DebugRenderer::submit_translate_gizmo(Math::fVec3 pos, i32 cx, i32 cy) {
		submit_quad(pos, {1.f, 0.f, 0.f}, {0.f, 0.f, .1f}, {1.f, 0.f, 0.f}, (u32)SelectedAxis::X);
		submit_quad(pos, {.1f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, (u32)SelectedAxis::Y);
		submit_quad(pos, {.1f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, (u32)SelectedAxis::Z);
		submit_quad(pos + Math::fVec3{0.2f, 0.2f, 0.f}, {.4f, 0.f, 0.f}, {0.f, .4f, 0.f}, {1.f, 1.f, 0.f}, (u32)SelectedAxis::XY);
		submit_quad(pos + Math::fVec3{0.2f, 0.f, 0.2f}, {.4f, 0.f, 0.f}, {0.f, 0.f, .4f}, {1.f, 0.f, 1.f}, (u32)SelectedAxis::XZ);
		submit_quad(pos + Math::fVec3{0.f, 0.2f, 0.2f}, {.0f, 0.4f, 0.f}, {0.f, 0.f, .4f}, {0.f, 1.f, 1.f}, (u32)SelectedAxis::YZ);
		return (SelectedAxis)current_id(cx, cy);
	}

	void DebugRenderer::draw_grid(i32 w, i32 d) {
		for (i32 x = -w; x <= w; x++) {
			submit_quad({(float)x, 0.f, (float)-w}, {0.1f, 0.f, 0.f}, {0.f, 0.f, 2.f * w}, {0.2f, 0.2f, 0.2f}, 0);
		}
		for (i32 z = -d; z <= d; z++) {
			submit_quad({(float)-d, 0.f, (float)z}, {0.f, 0.f, 0.1f}, {2.f * d, 0.f, 0.f}, {0.2f, 0.2f, 0.2f}, 0);
		}
	}

	void DebugRenderer::update(App::Scene& scene, ECS::Manager& mgr, float dt) {
		mShader.bind();
		auto& cam = scene.camera();
		Math::fMat4 proj = cam.projection();
		Math::fMat4 view = cam.view();
		
		mShader.set_uniform_m4("sys_proj", proj);
		mShader.set_uniform_m4("sys_view", view);

		glDisable(GL_CULL_FACE);
		
		u32 vao{}, vbo{};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(DebugVertex), mVertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(DebugVertex), (const void*)offsetof(DebugVertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(DebugVertex), (const void*)offsetof(DebugVertex, color));

		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(DebugVertex), (const void*)offsetof(DebugVertex, id));

		{
			mRenderTarget.bind();
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			u32 bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(2, bufs);
			
			glDrawArrays(GL_TRIANGLES, 0, (u32)mVertices.size());
			
			mRenderTarget.unbind();
		}
		{
			glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
		}

		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);

		mVertices.clear();
	}
	
	u32 DebugRenderer::current_id(i32 x, i32 y) { 
		if(x < 0) return 0;
		if(y < 0) return 0;
		if(x >= (i32)mRenderTarget.width()) return 0;
		if(y >= (i32)mRenderTarget.height()) return 0;

		return mRenderTarget.read_from_buffer_u32(1, x, y);
	}

}
