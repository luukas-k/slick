#include "Slick.h"

#include "MeshFormat/GLTFLoader.h"
#include "stb_image.h"
#include "Editor.h"


using namespace Slick;
using namespace Slick::Editor;

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

Format format_from_ctype_and_type(GLTFElementType ct, GLTFType t) {
	if (ct == GLTFElementType::FLOAT) {
		if (t == GLTFType::VEC4) {
			return Format::Float4;
		}
		else if (t == GLTFType::VEC3) {
			return Format::Float3;
		}
		else if (t == GLTFType::VEC2) {
			return Format::Float2;
		}
	}
	else if (ct == GLTFElementType::UNSIGNED_SHORT) {
		if (t == GLTFType::SCALAR) {
			return Format::UInt16;
		}
	}
	Utility::Assert(false);
	return Format::Unknown;
};



struct BoundingBox {
	Math::fVec3 min, max;

	float sq_distance(Math::fVec3 p) {
		Math::fVec3 p2{
			Math::clamp(min.x, max.x, p.x),
			Math::clamp(min.y, max.y, p.y),
			Math::clamp(min.z, max.z, p.z)
		};
		float dx = p2.x - p.x;
		float dy = p2.y - p.y;
		float dz = p2.z - p.z;
		return dx * dx + dy * dy + dz * dz;
	}
};

struct Message {
	u32 a, b;
};

EditorLayer::EditorLayer()
	:
	mProgram("shader/vs.glsl", "shader/fs.glsl"),
	mActiveEntity(0), mFrameDelta(0.f), mLastRender(0.f) {
	Utility::register_log_handler([&](const std::string& msg) {
		mLogHistory.push_back("[" + format(mTimer.elapsed()) + "]: " + msg);
	});

	mConnection.register_type<Message>(1);

	mConnection.on<Message>([](const Message& msg) {
		Utility::Log("client", msg.a, msg.b);
	});

	auto& mgr = mEditorScene.manager();

	UI::create_context();

	auto& cam = mEditorScene.camera();
	cam.set_position({ 0.f, 0.f, 3.f });

	// auto gun_gltf = Editor::load_gltf("model/gun/gun.gltf");

	auto gltf = Editor::load_gltf("model/sponza.gltf");
	// auto gltf = Editor::load_gltf("model/bollard.gltf");

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

			PBRMaterial material{
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
				material.baseColorTexture = texid;
			}
			if (mat.metallic_roughness_texture != -1) {
				auto& tex = gltf.textures[mat.metallic_roughness_texture];
				u32 texid = texture_ids[tex.source];
				material.metallicRoughnessTexture = texid;
			}
			if (mat.normal_texture != -1) {
				auto& tex = gltf.textures[mat.normal_texture];
				u32 texid = texture_ids[tex.source];
				material.normalTexture = texid;
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

			if (prim.attribute_tangent != -1) {
				auto& tangAcc = gltf.accessors[prim.attribute_tangent];
				auto& tangBufView = gltf.buffer_views[tangAcc.buffer_view];

				rc.tangentBuffer = buffer_ids[tangBufView.buffer];
				rc.tangentOffset = tangBufView.offset;
				rc.tangentFormat = format_from_ctype_and_type(tangAcc.component_type, tangAcc.type);
			}
			else {
				rc.tangentBuffer = -1;
				rc.tangentOffset = -1;
				rc.tangentFormat = Format::Unknown;
			}

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
			tf->position = { 0.f, 0.f, 0.f };
			RenderableComponent* rcc = mgr.add_component<RenderableComponent>(ent);
			rcc->mesh = (u32)mRenderCommands.size();
			rcc->material = (u32)mMaterials.size();

			mRenderCommands.push_back(rc);
			mMaterials.push_back(material);
		}
	}

	mLastUpdate = (float)mTimer.elapsed();
}

Slick::Editor::EditorLayer::~EditorLayer() {
	UI::destroy_ui();

	Utility::unregister_log_handler();
}

void EditorLayer::update(App::Application& app) {
	float currentTime = (float)mTimer.elapsed();
	float dt = 1.f / 100.f;
	while (mLastUpdate + dt < currentTime) {
		fixed_update(dt);
		mLastUpdate += dt;
	}
	mFrames++;
}

void EditorLayer::fixed_update(float dt) {
	mInput.update();
	mEditorScene.update(dt);

	Math::fVec3 movement{
		mInput.key_state(Input::Key::Key_A) * -1.f + mInput.key_state(Input::Key::Key_D) * 1.f,
		mInput.key_state(Input::Key::Key_Shift) * -1.f + mInput.key_state(Input::Key::Key_Space) * 1.f,
		mInput.key_state(Input::Key::Key_S) * 1.f + mInput.key_state(Input::Key::Key_W) * -1.f
	};

	float sensitivity = 0.01f;
	float speed = mInput.key_state(Input::Key::Key_Ctrl) ? 20.f : 5.f;
	auto& cam = mEditorScene.camera();
	cam.translate_local(movement * speed * dt);

	if (mInput.button_state(Input::Button::Button_Left))
		cam.rotate(Math::fVec3{ (float)mInput.cursor_dy() * sensitivity, (float)-mInput.cursor_dx() * sensitivity, 0.f });
}

void EditorLayer::render(App::Application& app, i32 w, i32 h) {
	float current = (float)mTimer.elapsed();
	float dt = current - mLastRender;
	mLastRender = current;
	mFrameDelta = dt;

	auto data = UI::get_ui_data();
	data->vp = { 0, 0, w, h };

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

	u32 i = 0;
	mEditorScene.manager().view<TransformComponent, LightComponent>([&](u32 ent, TransformComponent* tc, LightComponent* lc) {
		mProgram.set_uniform_f3("light_positions[" + std::to_string(i) + "]", tc->position);
		mProgram.set_uniform_f3("light_colors[" + std::to_string(i) + "]", lc->color);
		i++;
		mProgram.set_uniform_i1("light_count", i);
	});

	mEditorScene.manager().view<TransformComponent, RenderableComponent>([&](u32 ent, TransformComponent* tc, RenderableComponent* rrc) {
		if (rrc->mesh >= mRenderCommands.size())
			return;

		auto& rc = mRenderCommands[rrc->mesh];
		auto& mat = mMaterials[rrc->material];

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

		if (mat.baseColorTexture == -1) {
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

		auto enable_attribute = [](u32 index, i32 buffer, Format fmt, u32 offset) {
			if (buffer == -1) return;

			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, format_count(fmt), format_type(fmt), false, 0, (const void*)offset);
		};

		enable_attribute(0, rc.posBuffer, rc.posFormat, rc.posOffset);
		enable_attribute(1, rc.normalBuffer, rc.normalFormat, rc.normalOffset);
		enable_attribute(2, rc.tangentBuffer, rc.tangentFormat, rc.tangentOffset);
		enable_attribute(3, rc.uvBuffer, rc.uvFormat, rc.uvOffset);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.indexBuffer);
		glDrawElements(GL_TRIANGLES, (u32)rc.indexCount, format_type(rc.indexFormat), (const void*)(intptr_t)rc.indexOffset);
	});

	UI::frame([&]() {
		// UI::root_dockarea();

		UI::window("Primary window", [&]() {
			UI::container("Tools", [&]() {
				if (UI::button("Add light.")) {
					Utility::Log("Create light.");

					auto& mgr = mEditorScene.manager();
					u32 ent = mgr.create();
					TransformComponent* tc = mgr.add_component<TransformComponent>(ent);
					tc->position = mEditorScene.camera().pos();
					LightComponent* lc = mgr.add_component<LightComponent>(ent);
					lc->color = { 1.f, 1.f, 1.f };
				}
			});
		});
		UI::window("Secondary window", [&]() {
			UI::container("Entities", [&]() {
				mEditorScene.manager().view([&](u32 ent) {
					if (UI::button("Entity (" + std::to_string(ent) + ")")) {
						mActiveEntity = ent;
					}
				});
			});
		});
		UI::window("Entity panel", [&]() {
			UI::container("Entity", [&]() {
				if (mActiveEntity == 0) {
					UI::button("No active ent");
					return;
				}
				
				if (UI::button("Delete")) {
					mEditorScene.manager().destroy(mActiveEntity);
					mActiveEntity = 0;
				}

				auto tf = mEditorScene.manager().get_component<TransformComponent>(mActiveEntity);
				if (tf) {
					UI::container("Transform", [&]() {
						UI::button(format(tf->position));
					});
				}
			});
		});
		UI::window("Network window", [&]() {
			UI::container("Client", [&]() {
				if (!mConnection.is_connected()) {
					if (UI::button("Connect")) {
						mConnection.connect("127.0.0.1", 5232);
					}
				}
				else {
					if (UI::button("Send hello")) {
						// mConnection.send("hello");
						mConnection.send<Message>(Message{
							.a = 5,
							.b = 16
						});
					}
					if (UI::button("Disconnect")) {
						mConnection.disconnect();
					}
				}
			});
			UI::container("Server", [&]() {
				if (!app.get_layer<ServerLayer>()->is_active()) {
					if (UI::button("Start")) {
						app.get_layer<ServerLayer>()->start_server();
					}
				}
				else {
					if (UI::button("Stop")) {
						app.get_layer<ServerLayer>()->stop_server();
					}
				}
			});
		});
		UI::window("Log", [&]() {
			for (auto& msg : mLogHistory) {
				UI::button(msg);
			}
		});
		UI::window("Performance", [&]() {
			UI::button("DT:  " + format(mFrameDelta));
			static float t = 0.f;
			UI::button("FPS: " + format(1.f / mFrameDelta));
			if(UI::button("FPS: " + format(t))) {
				t = 1.f / mFrameDelta;
			}
		});
	});

	data->scroll_x = 0;
	data->scroll_y = 0;
}

void EditorLayer::on_key(Input::Key kc, bool state) {
	mInput.on_key(kc, state);
}

void EditorLayer::on_button(Input::Button kc, bool state) {
	mInput.on_button(kc, state);
	auto data = UI::get_ui_data();
	if (kc == Input::Button::Button_Left)
		data->clicked = state;
}

void EditorLayer::on_cursor_move(i32 x, i32 y) {
	auto data = UI::get_ui_data();
	data->cx = x;
	data->cy = y;
	mInput.on_cursor_move(x, y);
}

void EditorLayer::on_scroll(i32 x, i32 y) {
	auto data = UI::get_ui_data();
	data->scroll_x += x;
	data->scroll_y += y;
}

ServerLayer::ServerLayer() 
{
	mServer.register_type<Message>(1);

	mServer.on<Message>([&](const Message& m, u32 conn_id) {
		Utility::Log("Server", m.a, m.b, "From", conn_id);
		mServer.send<Message>({ .a = m.a + 1, .b = m.b + 1 }, conn_id);
	});
}

ServerLayer::~ServerLayer() {}

// Layer

void ServerLayer::update(App::Application& app) {
	if (mServer.is_active()) {
	}
}

void ServerLayer::render(App::Application& app, i32 w, i32 h) {}

void ServerLayer::on_cursor_move(i32 w, i32 h) {}

void ServerLayer::on_key(Input::Key kc, bool state) {}

void ServerLayer::on_button(Input::Button kc, bool state) {}

// Server

void ServerLayer::start_server() {
	mServer.listen(5232);
}

void ServerLayer::stop_server() {
	mServer.stop();
}

bool ServerLayer::is_active() {
	return mServer.is_active();
}


int main() {
	App::Application app;

	app.create_layer<ServerLayer>("ServerLayer");
	app.create_layer<EditorLayer>("EditorLayer");

	app.run();

	return 0;
}
