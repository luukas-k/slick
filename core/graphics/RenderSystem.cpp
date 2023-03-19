#include "RenderSystem.h"

#include "glad/glad.h"

namespace Slick::Gfx {

	RenderSystem::RenderSystem(Camera& cam, App::ResourceManager& resources)
		:
		mCamera(cam),
		mResources(resources),
		mProgram("shader/vs.glsl", "shader/fs.glsl") {

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

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

	void RenderSystem::update(ECS::Manager& mgr, float dt) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);

		auto& cam = mCamera;
		Math::fMat4 proj = cam.projection();
		Math::fMat4 view = cam.view();

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
			const auto& rc = mResources.get_mesh_by_id(rrc->mesh);
			const auto& mat = mResources.get_material_by_id(rrc->material);

			Math::fMat4 model = Math::translation(tc->position) * Math::scale({ 0.00800000037997961f, 0.00800000037997961f, 0.00800000037997961f });
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
	}

}
