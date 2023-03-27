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

	void DebugRenderer::submit_quad(Math::fVec3 pos, Math::fVec3 axis0, Math::fVec3 axis1, Math::fVec3 color) {
		std::vector<Math::fVec3> positions{
			pos + axis0 * -1.f + axis1 * -1.f,
			pos + axis0 * -1.f + axis1 *  1.f,
			pos + axis0 *  1.f + axis1 *  1.f,

			pos + axis0 *  1.f + axis1 *  1.f,
			pos + axis0 *  1.f + axis1 * -1.f,
			pos + axis0 * -1.f + axis1 * -1.f
		};

		for (auto& v : positions) {
			mVertices.push_back({ v, color });
		}
	}

	void DebugRenderer::update(App::Scene& scene, ECS::Manager& mgr, float dt) {
		mShader.bind();
		auto& cam = scene.camera();
		Math::fMat4 proj = cam.projection();
		Math::fMat4 view = cam.view();
		
		mShader.set_uniform_m4("sys_proj", proj);
		mShader.set_uniform_m4("sys_view", view);

		
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

		{
			mRenderTarget.bind();
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			u32 bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(2, bufs);
			
			glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
			
			mRenderTarget.unbind();
		}
		{
			glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
		}

		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);

		mVertices.clear();
	}
	
	u32 DebugRenderer::current_id(i32 x, i32 y) { return mRenderTarget.read_from_buffer_u32(1, x, y); }

}
