#include "RenderSystem.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Slick::Gfx {

	RenderSystem::RenderSystem()
		:
		mProgram("shader/vs.glsl", "shader/fs.glsl"),
		mSkyShader("shader/vs_sky.glsl", "shader/fs_sky.glsl"),
		mScreen({0,0,1920,1080}),
		mRenderTarget(1920, 1080, { { TextureFormat::RGBA }, { TextureFormat::Depth } })
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		u32 cube_map{};
		glGenTextures(1, &cube_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		auto load_face = [](const std::string& fname, u32 i) {
			i32 w{}, h{}, c{};
			u8* data = stbi_load(fname.c_str(), &w, &h, &c, 4);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		};

		load_face("skybox/right.jpg", 0);
		load_face("skybox/left.jpg", 1);
		load_face("skybox/top.jpg", 2);
		load_face("skybox/bottom.jpg", 3);
		load_face("skybox/front.jpg", 4);
		load_face("skybox/back.jpg", 5);

		mSkybox = cube_map;
	}

	RenderSystem::~RenderSystem() {}

	u32 format_type(App::Format f) {
		switch (f) {
			case App::Format::Float4: return GL_FLOAT;
			case App::Format::Float3: return GL_FLOAT;
			case App::Format::Float2: return GL_FLOAT;
			case App::Format::Float1: return GL_FLOAT;
			case App::Format::UInt16: return GL_UNSIGNED_SHORT;
		}
		Utility::Assert(false);
		return 0;
	}

	u32 format_count(App::Format f) {
		switch (f) {
			case App::Format::Float4: return 4;
			case App::Format::Float3: return 3;
			case App::Format::Float2: return 2;
			case App::Format::Float1: return 1;
			case App::Format::UInt16: return 1;
		}
		Utility::Assert(false);
		return 0;
	}

	void RenderSystem::update(App::Scene& scene, ECS::Manager& mgr, float dt) {
		auto& cam = scene.camera();
		Math::fMat4 proj = cam.projection();
		Math::fMat4 view = cam.view();
		
		// Render skybox
		{
			glDisable(GL_CULL_FACE);

			mSkyShader.bind();
			mSkyShader.set_uniform_m4("sys_proj", proj);
			mSkyShader.set_uniform_m4("sys_view", view);

			u32 vao{}, vbo{};
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mSkybox);
			mSkyShader.set_uniform_i1("tex_skybox", 0);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			float s = 250.f;
			Math::fVec3 positions[] = {
				// Back
				{ -s, -s, -s },
				{ -s,  s, -s },
				{  s,  s, -s },

				{  s,  s, -s },
				{  s, -s, -s },
				{ -s, -s, -s },
				
				// Front
				{ -s, -s,  s },
				{ -s,  s,  s },
				{  s,  s,  s },

				{  s,  s,  s },
				{  s, -s,  s },
				{ -s, -s,  s },
				
				// Left
				{ -s, -s, -s },
				{ -s, -s,  s },
				{ -s,  s,  s },

				{ -s,  s,  s },
				{ -s,  s, -s },
				{ -s, -s, -s },

				// Right
				{  s, -s, -s },
				{  s, -s,  s },
				{  s,  s,  s },

				{  s,  s,  s },
				{  s,  s, -s },
				{  s, -s, -s },
				
				// Top
				{  -s, s, -s },
				{  -s, s,  s },
				{   s, s,  s },

				{   s, s,  s },
				{   s, s, -s },
				{  -s, s, -s },
				
				// Bottom
				{  -s, -s, -s },
				{  -s, -s,  s },
				{   s, -s,  s },

				{   s, -s,  s },
				{   s, -s, -s },
				{  -s, -s, -s },
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Math::fVec3), 0);

			glDrawArrays(GL_TRIANGLES, 0, sizeof(positions) / sizeof(positions[0]));

			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
		}
		glEnable(GL_CULL_FACE);

		// Render world
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);

		auto& resources = scene.resources();

		mProgram.bind();

		mProgram.set_uniform_f3("cam_pos", cam.pos());

		mProgram.set_uniform_m4("sys_proj", proj);
		mProgram.set_uniform_m4("sys_view", view);

		glBindVertexArray(vao);

		u32 i = 0;
		mgr.view<TransformComponent, LightComponent>([&](u32 ent, TransformComponent* tc, LightComponent* lc) {
			mProgram.set_uniform_f3("light_positions[" + std::to_string(i) + "]", tc->position);
			mProgram.set_uniform_f3("light_colors[" + std::to_string(i) + "]", lc->color);
			i++;
			mProgram.set_uniform_i1("light_count", i);
		});

		mgr.view<TransformComponent, RenderableComponent>([&](u32 ent, TransformComponent* tc, RenderableComponent* rrc) {
			const auto& rc = resources.get_mesh_by_id(rrc->mesh);
			const auto& mat = resources.get_material_by_id(rrc->material);

			Math::fMat4 tx = Math::translation(tc->position),
				sc = Math::scale(tc->scale),
				rot = Math::rotation_matrix(tc->rotation);

			Math::fMat4 model = sc * rot * tx;
			mProgram.set_uniform_m4("sys_model", model);

			if (mat.baseColorTexture == -1) {
				// No texture
				mProgram.set_uniform_i1("mat_has_base_color_texture", 0);
				mProgram.set_uniform_f3("mat_base_color", mat.baseColor);
			}
			else {
				// Has texture
				u32 index = 0;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, mat.baseColorTexture);
				mProgram.set_uniform_i1("mat_base_color_texture", index);

				mProgram.set_uniform_i1("mat_has_base_color_texture", 1);
			}

			if (mat.metallicRoughnessTexture == -1) {
				// No texture
				mProgram.set_uniform_i1("mat_has_metallic_roughness", 0);
				mProgram.set_uniform_f1("mat_metallic", mat.metallic);
				mProgram.set_uniform_f1("mat_roughness", mat.roughness);
			}
			else {
				// Has texture
				u32 index = 1;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, mat.metallicRoughnessTexture);
				mProgram.set_uniform_i1("mat_metallic_roughness_texture", index);

				mProgram.set_uniform_i1("mat_has_metallic_roughness", 1);
			}

			if (mat.normalTexture == -1) {
				mProgram.set_uniform_i1("mat_has_normal", 0);
			}
			else {
				u32 index = 2;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, mat.normalTexture);
				mProgram.set_uniform_i1("mat_normal_map", index);

				mProgram.set_uniform_i1("mat_has_normal", 1);
			}

			auto enable_attribute = [](u32 index, App::BufferReference buf) {
				glBindBuffer(GL_ARRAY_BUFFER, buf.buffer);
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index, format_count(buf.fmt), format_type(buf.fmt), false, 0, (const void*)buf.offset);
			};

			enable_attribute(0, rc.position);
			if (rc.hasNormals)
				enable_attribute(1, rc.normal);
			if (rc.hasTangents)
				enable_attribute(2, rc.tangent);
			if (rc.hasUvs)
				enable_attribute(3, rc.uvs);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.indices.buffer);
			glDrawElements(GL_TRIANGLES, (u32)rc.drawCount, format_type(rc.indices.fmt), (const void*)(intptr_t)rc.indices.offset);
		});

		/*mRenderTarget.unbind();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, mRenderTarget.id());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, mScreen.w, mScreen.h, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);*/
	}

}
