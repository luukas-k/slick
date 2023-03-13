#include "Slick.h"

#include "MeshFormat/GLTFLoader.h"
#include "stb_image.h"

struct tf {
	float x, y, z;
};

namespace Slick {

	template<>
	std::string format<tf>(tf v) {
		return "(" + format(v.x) + " " + format(v.y) + " " + format(v.z) + ")";
	}

}

using namespace Slick;

enum struct Format : u8 {
	Unknown,

	Float4,
	Float3,
	Float2,
	Float1,
	UInt16,
};

u32 format_type(Format f) {
	switch (f) {
		case Format::Float4: return GL_FLOAT;
		case Format::Float3: return GL_FLOAT;
		case Format::Float2: return GL_FLOAT;
		case Format::Float1: return GL_FLOAT;
		case Format::UInt16: return GL_UNSIGNED_SHORT;
	}
	Utility::Assert(false);
	return 0;
}

u32 format_count(Format f) {
	switch (f) {
		case Format::Float4: return 4;
		case Format::Float3: return 3;
		case Format::Float2: return 2;
		case Format::Float1: return 1;
		case Format::UInt16: return 1;
	}
	Utility::Assert(false);
	return 0;
}

Format format_from_ctype_and_type(Editor::GLTFElementType ct, Editor::GLTFType t) {
	if (ct == Editor::GLTFElementType::FLOAT) {
		if (t == Editor::GLTFType::VEC4) {
			return Format::Float4;
		}
		else if (t == Editor::GLTFType::VEC3) {
			return Format::Float3;
		}
		else if (t == Editor::GLTFType::VEC2) {
			return Format::Float2;
		}
	}
	else if (ct == Editor::GLTFElementType::UNSIGNED_SHORT) {
		if (t == Editor::GLTFType::SCALAR) {
			return Format::UInt16;
		}
	}
	Utility::Assert(false);
	return Format::Unknown;
};

struct PBRMaterial {
	Math::fVec3 baseColor;
	i32 baseColorTexture;

	float metallic, roughness;
	i32 metallicRoughnessTexture;

	i32 normalTexture;
};

float clamp(float min, float max, float v) {
	if(v < min) return min;
	if(v > max) return max;
	return v;
}

struct BoundingBox {
	Math::fVec3 min, max;

	float sq_distance(Math::fVec3 p) {
		Math::fVec3 p2{
			clamp(min.x, max.x, p.x),
			clamp(min.y, max.y, p.y),
			clamp(min.z, max.z, p.z)
		};
		float dx = p2.x - p.x;
		float dy = p2.y - p.y;
		float dz = p2.z - p.z;
		return dx * dx + dy * dy + dz * dz;
	}
};

struct RenderCommand {
	u32 posBuffer, posOffset;
	Format posFormat;

	u32 normalBuffer, normalOffset;
	Format normalFormat;
	
	u32 tangentBuffer, tangentOffset;
	Format tangentFormat;

	u32 uvBuffer, uvOffset;
	Format uvFormat;

	u32 indexBuffer, indexOffset, indexCount;
	Format indexFormat;

	PBRMaterial material;
};

class EditorLayer {
public:
	EditorLayer()
		:
		mProgram("shader/vs.glsl", "shader/fs.glsl")
	{
		auto& mgr = mEditorScene.manager();
		
		UI::create_context();

		auto& cam = mEditorScene.camera();
		cam.set_position({0.f, 0.f, 20.f});

		auto gltf = Editor::load_gltf("model/sponza.gltf");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		std::vector<u32> buffer_ids;
		for (auto& buffer : gltf.buffers) {
			u32 id{};
			glGenBuffers(1, &id);
			glBindBuffer(GL_ARRAY_BUFFER, id);
			glBufferData(GL_ARRAY_BUFFER, buffer.data.size(), buffer.data.data(), GL_STATIC_DRAW);
			buffer_ids.push_back(id);
		}

		std::vector<u32> texture_ids;
		for (auto& img : gltf.images) {
			auto& bv = gltf.buffer_views[img.buffer_view];
			auto& b = gltf.buffers[bv.buffer];

			i32 w{}, h{}, c{};
			u8* img_data = stbi_load_from_memory(b.data.data() + bv.offset, bv.length, &w, &h, &c, 4);
			
			u32 texId{};
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)img_data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			stbi_image_free(img_data);

			texture_ids.push_back(texId);
		}


		for (auto& mesh : gltf.meshes) {
			for (auto& prim : mesh.primitives) {
				RenderCommand rc{};

				auto& mat = gltf.materials[prim.material];
				
				rc.material = PBRMaterial{
					.baseColor = mat.base_color,
					.baseColorTexture = -1,
					.metallic = mat.metallic,
					.roughness = mat.roughness,
					.metallicRoughnessTexture = -1,
					.normalTexture = -1,
				};

				if (mat.base_color_texture != -1) {
					auto& tex = gltf.textures[mat.base_color_texture];
					u32 texid = texture_ids[tex.source];
					rc.material.baseColorTexture = texid;
				}
				if (mat.metallic_roughness_texture != -1) {
					auto& tex = gltf.textures[mat.metallic_roughness_texture];
					u32 texid = texture_ids[tex.source];
					rc.material.metallicRoughnessTexture = texid;
				}
				if (mat.normal_texture != -1) {
					auto& tex = gltf.textures[mat.normal_texture];
					u32 texid = texture_ids[tex.source];
					rc.material.normalTexture = texid;
				}

				auto& posAcc = gltf.accessors[prim.attribute_position];
				auto& posBufView = gltf.buffer_views[posAcc.buffer_view];
				
				rc.posBuffer = buffer_ids[posBufView.buffer];
				rc.posOffset = posBufView.offset;
				rc.posFormat = format_from_ctype_and_type(posAcc.component_type, posAcc.type);

				auto& normAcc = gltf.accessors[prim.attribute_normal];
				auto& normBufView = gltf.buffer_views[normAcc.buffer_view];
				
				rc.normalBuffer = buffer_ids[normBufView.buffer];
				rc.normalOffset = normBufView.offset;
				rc.normalFormat = format_from_ctype_and_type(normAcc.component_type, normAcc.type);

				auto& tangAcc = gltf.accessors[prim.attribute_tangent];
				auto& tangBufView = gltf.buffer_views[tangAcc.buffer_view];
				
				rc.tangentBuffer = buffer_ids[tangBufView.buffer];
				rc.tangentOffset = tangBufView.offset;
				rc.tangentFormat = format_from_ctype_and_type(tangAcc.component_type, tangAcc.type);

				auto& uvAcc = gltf.accessors[prim.attribute_texcoord];
				auto& uvBufView = gltf.buffer_views[uvAcc.buffer_view];
				
				rc.uvBuffer = buffer_ids[uvBufView.buffer];
				rc.uvOffset = uvBufView.offset;
				rc.uvFormat = format_from_ctype_and_type(uvAcc.component_type, uvAcc.type);

				auto& indexAcc = gltf.accessors[prim.indices];
				auto& indexBufView = gltf.buffer_views[indexAcc.buffer_view];

				rc.indexBuffer = buffer_ids[indexBufView.buffer];
				rc.indexOffset = indexBufView.offset;
				rc.indexFormat = format_from_ctype_and_type(indexAcc.component_type, indexAcc.type);
				rc.indexCount = indexAcc.count;

				u32 ent = mgr.create();
				TransformComponent* tf = mgr.add_component<TransformComponent>(ent);
				tf->position = {0.f, 0.f, 0.f};
				RenderableComponent* rcc = mgr.add_component<RenderableComponent>(ent);
				rcc->render_command = (u32)mRenderCommands.size();

				mRenderCommands.push_back(rc);
			}
		}

		mLastUpdate = (float)mTimer.elapsed();

	}
	~EditorLayer(){}

	void update() {
		float currentTime = (float)mTimer.elapsed();
		float dt = 1.f / 100.f;
		while (mLastUpdate + dt < currentTime) {
			fixed_update(dt);
			mLastUpdate += dt;
		}
		// Utility::Log(mFrames / mTimer.elapsed(), mTimer.elapsed() / mFrames);
		mFrames++;
	}
	u32 mFrames{0};
	void fixed_update(float dt) {
		mInput.update();

		Math::fVec3 movement{
			mInput.key_state(Input::Key::Key_A) * -1.f + mInput.key_state(Input::Key::Key_D) * 1.f,
			mInput.key_state(Input::Key::Key_Shift) * -1.f + mInput.key_state(Input::Key::Key_Space) * 1.f,
			mInput.key_state(Input::Key::Key_S) *  1.f + mInput.key_state(Input::Key::Key_W) * -1.f
		};

		float sensitivity = 0.01f;
		float speed = mInput.key_state(Input::Key::Key_Ctrl) ? 20.f : 5.f;
		auto& cam = mEditorScene.camera();
		cam.translate_local(movement * speed * dt);

		if(mInput.button_state(Input::Button::Button_Left))	
			cam.rotate(Math::fVec3{(float)mInput.cursor_dy() * sensitivity, (float)-mInput.cursor_dx() * sensitivity, 0.f});
	}
	void render(i32 w, i32 h) {
		auto data = UI::get_ui_data();
		data->vp = {0, 0, w, h};

		glViewport(0, 0, w, h);
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		glEnable(GL_CULL_FACE);

		auto& cam = mEditorScene.camera();
		Math::fMat4 proj = cam.projection((float)w / h);
		Math::fMat4 view = cam.view();

		mProgram.bind();
		
		mProgram.set_uniform_f3("cam_pos", cam.pos());

		mProgram.set_uniform_m4("sys_proj", proj);
		mProgram.set_uniform_m4("sys_view", view);

		

		glBindVertexArray(vao);

		mEditorScene.manager().view<TransformComponent, RenderableComponent>([&](u32 ent, TransformComponent* tc, RenderableComponent* rrc) {
			if(rrc->render_command >= mRenderCommands.size())
				return;

			auto& rc = mRenderCommands[rrc->render_command];

			Math::fMat4 model = Math::translation(tc->position) * Math::scale({0.00800000037997961f, 0.00800000037997961f, 0.00800000037997961f});
			mProgram.set_uniform_m4("sys_model", model);

			if (rc.material.baseColorTexture == -1) {
				// No texture
				mProgram.set_uniform_i1("mat_has_base_color_texture", 0);
				mProgram.set_uniform_f3("mat_base_color", rc.material.baseColor);
			}
			else {
				// Has texture
				u32 index = 0;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, rc.material.baseColorTexture);
				mProgram.set_uniform_i1("mat_base_color_texture", index);

				mProgram.set_uniform_i1("mat_has_base_color_texture", 1);
			}

			if (rc.material.baseColorTexture == -1) {
				// No texture
				mProgram.set_uniform_i1("mat_has_metallic_roughness", 0);
				mProgram.set_uniform_f1("mat_metallic", rc.material.metallic);
				mProgram.set_uniform_f1("mat_roughness", rc.material.roughness);
			}
			else {
				// Has texture
				u32 index = 1;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, rc.material.metallicRoughnessTexture);
				mProgram.set_uniform_i1("mat_metallic_roughness_texture", index);

				mProgram.set_uniform_i1("mat_has_metallic_roughness", 1);
			}

			if (rc.material.normalTexture == -1) {
				mProgram.set_uniform_i1("mat_has_normal", 0);
			}
			else {
				u32 index = 2;
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, rc.material.normalTexture);
				mProgram.set_uniform_i1("mat_normal_map", index);

				mProgram.set_uniform_i1("mat_has_normal", 1);
			}

			auto enable_attribute = [](u32 index, u32 buffer, Format fmt, u32 offset) {
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index, format_count(fmt), format_type(fmt), false, 0, (const void*)offset);
			};

			enable_attribute(0, rc.posBuffer, rc.posFormat, rc.posOffset);
			enable_attribute(1, rc.normalBuffer, rc.normalFormat, rc.normalOffset);
			enable_attribute(2, rc.tangentBuffer, rc.tangentFormat, rc.tangentOffset);
			enable_attribute(3, rc.uvBuffer, rc.uvFormat, rc.uvOffset);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.indexBuffer);
			glDrawElements(GL_TRIANGLES, rc.indexCount, format_type(rc.indexFormat), (const void*)rc.indexOffset);
		});

		UI::frame([&]() {
			UI::window("Window0", [&]() {
				UI::container("cont1", [&]() {
					if (UI::button("Set light to cam pos.")) {
						glUseProgram(mProgram.id());
						mProgram.set_uniform_f3("light_pos", cam.pos());
					}
					float v{(sinf(mLastUpdate) + 1.f) * 0.5f * 10.f};
					UI::slider("Hello", 0.f, 10.f, v);
				});
				UI::container("cont2", [&]() {
					if (UI::button("Toggle normal maps.")) {
						/*mEditorScene.manager().view<TransformComponent>([](u32 eid, TransformComponent* tc) {
							auto r = [](){ return (float)rand() / RAND_MAX; };
							tc->position = {r() * 10.f, r() * 10.f, r() * 10.f};
						});*/
					}
					if (UI::button("Hello2")) {
						Utility::Log("HelloC");
					}
				});
			});
		});
	}
	void on_key(Input::Key kc, bool state) {
		mInput.on_key(kc, state);
	}
	void on_button(Input::Button kc, bool state) {
		mInput.on_button(kc, state);
		auto data = UI::get_ui_data();
		if(kc == Input::Button::Button_Left)
			data->clicked = state;
	}
	void on_cursor_move(i32 x, i32 y) {
		auto data = UI::get_ui_data();
		data->cx = x;
		data->cy = y;
		mInput.on_cursor_move(x, y);
	}
private:
	App::Scene mEditorScene;
	Input::InputManager mInput;
	Utility::Timer mTimer;
	float mLastUpdate;
	u32 vao{};
	std::vector<RenderCommand> mRenderCommands;
	Gfx::Shader mProgram;
};

int main() {
	App::Application app;

	EditorLayer* layer = new EditorLayer();
	app.add_layer("EditorLayer", layer);

	app.run();

	return 0;
}