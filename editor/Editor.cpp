#include "Slick.h"

#include "stb_image.h"
#include "Editor.h"


using namespace Slick;
using namespace Slick::Editor;

struct Message {
	u32 a, b;
};

EditorLayer::EditorLayer()
	:
	mActiveEntity(0),
	mEditorScene(mResources),
	mSensitivity(0.05),
	mShowUI(true)
{
	mEditorScene.register_system_dynamic(mRenderer);
	mEditorScene.register_system_fixed(mPhysics);

	Utility::register_log_handler([&](const std::string& msg) {
		mLogHistory.push_back("[" + format(mTimer.elapsed()) + "]: " + msg);
	});

	mConnection.register_type<Message>(1);

	mConnection.on<Message>([](const Message& msg) {
		Utility::Log("client", msg.a, msg.b);
	});

	UI::create_context();

	auto& cam = mEditorScene.camera();
	cam.set_position({ 0.f, 0.f, .5f });

	
}

Slick::Editor::EditorLayer::~EditorLayer() {
	UI::destroy_ui();

	Utility::unregister_log_handler();
}


void entity_panel(App::Scene& scene, u32& active_entity) {
	UI::window("Entity panel", [&](){
		UI::container("Entity", [&]() {
			if (active_entity == 0) {
				UI::label("No active ent");
				return;
			}

			if (UI::button("Delete")) {
				scene.destroy_entity(active_entity);
				active_entity = 0;
			}

			auto tf = scene.get_component<TransformComponent>(active_entity);
			if (tf) {
				UI::container("Transform", [&]() {
					UI::slider("x", -100.f, 100.f, tf->position.x);
					UI::slider("y", -100.f, 100.f, tf->position.y);
					UI::slider("z", -100.f, 100.f, tf->position.z);
				});
			}
			auto lc = scene.get_component<LightComponent>(active_entity);
			if (lc) {
				UI::container("Light", [&]() {
					UI::slider("r", 0.f, 1.f, lc->color.x);
					UI::slider("g", 0.f, 1.f, lc->color.y);
					UI::slider("b", 0.f, 1.f, lc->color.z);
				});
			}
		});
	});
}

void scene_hierarchy_panel(App::Scene& scene, u32& active_entity) {
	UI::window("Scene hierarchy", [&]() {
		UI::container("Entities", [&]() {
			scene.view([&](u32 ent) {
				if (UI::button(scene.get_name(ent))) {
					active_entity = ent;
				}
			});
		});
	});
}

void tool_panel(EditorLayer& editor) {
	UI::window("Primary window", [&]() {
		UI::container("Tools", [&]() {
			if (UI::button("Reset Camera Position")) {
				editor.active_scene().camera().set_position({0.f, 5.f, 0.f});
			}
			if (UI::button("Add light.")) {
				Utility::Log("Create light.");

				auto [ent, tc, lc] = editor.active_scene().create_entity<TransformComponent, LightComponent>("Name");
				tc->position = editor.active_scene().camera().pos();
				tc->rotation = { 0.f, 0.f, 0.f, 1.f };
				lc->color = { 1.f, 1.f, 1.f };
			}
			if (UI::button("Load")) {
				editor.load_to_scene("model/sponza.gltf", editor.active_scene().camera().pos());
			}
		});
	});
}

void camera_panel(Gfx::Camera& cam, float& sens) {
	UI::window("Camera", [&]() {
		UI::container("Position", [&]() {
			UI::slider("x", -100.f, 100.f, cam.pos().x);
			UI::slider("y", -100.f, 100.f, cam.pos().y);
			UI::slider("z", -100.f, 100.f, cam.pos().z);
		});
		UI::container("Fov", [&]() {
			float fov = cam.fov() * (180.f / 3.1415f);
			UI::slider("fov", 0.f, 180.f, fov);
			cam.fov() = fov * (3.1415f / 180.f);
		});
		UI::container("Sensitivity", [&]() {
			UI::slider("sens", 0.f, 1.f, sens);
		});
	});
}

void audio_panel(Audio::AudioDevice& audio) {
	UI::window("Audio", [&]() {
		if (UI::button("Mute")) {
			audio.mute();
		}
		if (UI::button("Unmute")) {
			audio.unmute();
		}
		UI::slider("Volume", 0.f, 1.f, audio.volume());
	});
}

void network_panel() {
	/*UI::window("Network window", [&]() {
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
	});*/
}

void console_log(const std::vector<std::string>& log_history) {
	UI::window("Log", [&]() {
		for (u32 i = 0; i < log_history.size(); i++) {
			auto& msg = log_history[log_history.size() - 1 - i];
			UI::label(msg);
		}
	});
}

void EditorLayer::update(App::Application& app) {
	float dt = (float)mTimer.elapsed();
	mTimer.reset();
	
	mRenderer.set_viewport({ 0, 0, app.surface().width(), app.surface().height() });
	mQueue.run_commands();

	mInput.update();
	
	Math::fVec3 movement{
		mInput.key_state(Input::Key::Key_A) * -1.f + mInput.key_state(Input::Key::Key_D) * 1.f,
		mInput.key_state(Input::Key::Key_Shift) * -1.f + mInput.key_state(Input::Key::Key_Space) * 1.f,
		mInput.key_state(Input::Key::Key_S) * 1.f + mInput.key_state(Input::Key::Key_W) * -1.f
	};

	float speed = mInput.key_state(Input::Key::Key_Ctrl) ? 5.f : 1.f;
	auto& cam = mEditorScene.camera();
	cam.translate_local(movement * speed * dt);
	
	if (mInput.button_state(Input::Button::Button_Left))
		cam.rotate(Math::fVec3{ (float)mInput.cursor_dy() * mSensitivity, (float)-mInput.cursor_dx() * mSensitivity, 0.f });

	auto data = UI::get_ui_data();
	i32 w = app.surface().width();
	i32 h = app.surface().height();
	data->vp = { 0, 0, w, h };
	cam.set_aspect_ratio((float)w / h);

	glViewport(0, 0, w, h);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mEditorScene.update(dt);

	if (mShowUI) {
		UI::frame([&]() {
			tool_panel(*this);
			entity_panel(mEditorScene, mActiveEntity);
			scene_hierarchy_panel(mEditorScene, mActiveEntity);
			camera_panel(mEditorScene.camera(), mSensitivity);
			audio_panel(mAudio);
			console_log(mLogHistory);
			UI::window("Scene", [&]() {
				if (UI::button("Save")) {
					mEditorScene.save_scene("scene.scene");
				}
			});
		});
	}
}

void EditorLayer::on_key(Input::Key kc, bool state) {
	mInput.on_key(kc, state);

	if (kc == Input::Key::Key_P && !state) {
		mShowUI = !mShowUI;
	}
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

void Slick::Editor::EditorLayer::load_to_scene(const std::string& fname, Math::fVec3 pos) {
	mPool.submit_command([this, fn = fname, p = pos]() {
		auto gltf = Loader::load_gltf(fn);

		mQueue.submit_command([this, m = gltf, p = p]() {
			auto meshes = mResources.generate_meshes_from_gltf(m);
				
			for (u32 i = 0; i < meshes.size(); i++) {
				auto&[mesh, mat] = meshes[i];

				auto[ent, tc, rc] = mEditorScene.create_entity<TransformComponent, RenderableComponent>("Object " + format(i));
				tc->position = p;
				tc->scale = { 0.008f, 0.008f, 0.008f };
				tc->rotation = { 0.f, 0.f, 0.f, 1.f };
				rc->mesh = mesh;
				rc->material = mat;
			}
		});
	});
}

ServerLayer::ServerLayer() 
	:
	mScene(mResources)
{
	mServer.register_type<Message>(1);

	mServer.on_connect([](u32 conn_id) {
		Utility::Log("Connected: ", conn_id);
	});

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
