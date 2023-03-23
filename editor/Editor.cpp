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
	mActiveEntity(0), mFrameDelta(0.f), mLastRender(0.f),
	mEditorScene(mResources)
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

	auto load_mesh_to_scene = [&](const std::string& fname, Math::fVec3 pos, bool phys) {
		auto meshes = mResources.load_mesh(fname);
		for (auto& [mesh, mat] : meshes) {
			if (!phys) {
				auto[ent, tc, rc] = mEditorScene.create_entity<TransformComponent, RenderableComponent>();
				tc->position = pos;
				tc->scale = { 0.00800000037997961f, 0.00800000037997961f, 0.00800000037997961f };
				tc->rotation = { 0.f, 0.f, 0.f, 1.f };
				rc->mesh = mesh;
				rc->material = mat;
			}
			else {
				auto[ent, tc, rc, rb, sc] = mEditorScene.create_entity<TransformComponent, RenderableComponent, RigidBody, SphereCollider>();
				tc->position = pos;
				tc->scale = { 0.00800000037997961f, 0.00800000037997961f, 0.00800000037997961f };
				tc->rotation = { 0.f, 0.f, 0.f, 1.f };
				rc->mesh = mesh;
				rc->material = mat;
				sc->radius = 1.f;
			}
		}
	};

	// fname "model/bollard.gltf"

	// load_mesh_to_scene("model/sponza.gltf");
	for (u32 i = 0; i < 100; i++) {
		Math::fVec3 off{
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX
		};
		off = off * 2.f - 1.f;
		off = off * 0.1f;
		load_mesh_to_scene("model/bollard.gltf", {off.x, (float)i * 5.f, off.z}, true);
	}

	mLastUpdate = (float)mTimer.elapsed();
}

Slick::Editor::EditorLayer::~EditorLayer() {
	UI::destroy_ui();

	Utility::unregister_log_handler();
}

void EditorLayer::update(App::Application& app) {
	float currentTime = (float)mTimer.elapsed();
	float dt = currentTime - mLastUpdate;

	mInput.update();
	
	Math::fVec3 movement{
		mInput.key_state(Input::Key::Key_A) * -1.f + mInput.key_state(Input::Key::Key_D) * 1.f,
		mInput.key_state(Input::Key::Key_Shift) * -1.f + mInput.key_state(Input::Key::Key_Space) * 1.f,
		mInput.key_state(Input::Key::Key_S) * 1.f + mInput.key_state(Input::Key::Key_W) * -1.f
	};

	float sensitivity = 0.01f;
	float speed = mInput.key_state(Input::Key::Key_Ctrl) ? 5.f : 1.f;
	auto& cam = mEditorScene.camera();
	cam.translate_local(movement * speed * dt);
	
	if (mInput.button_state(Input::Button::Button_Left))
		cam.rotate(Math::fVec3{ (float)mInput.cursor_dy() * sensitivity, (float)-mInput.cursor_dx() * sensitivity, 0.f });

	auto data = UI::get_ui_data();
	i32 w = app.surface().width();
	i32 h = app.surface().height();
	data->vp = { 0, 0, w, h };
	cam.set_aspect_ratio((float)w / h);

	glViewport(0, 0, w, h);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mEditorScene.update(dt);

	mLastUpdate = currentTime;

	UI::frame([&]() {
		UI::window("Primary window", [&]() {
			UI::container("Tools", [&]() {
				if (UI::button("Reset pos")) {
					auto& mgr = mEditorScene.manager();
					mgr.view<TransformComponent>([](u32 e, TransformComponent* tc) {
						tc->position = {0.f, 5.f, 0.f};
					});
				}
				if (UI::button("Add light.")) {
					Utility::Log("Create light.");

					auto& mgr = mEditorScene.manager();
					u32 ent = mgr.create();
					TransformComponent* tc = mgr.add_component<TransformComponent>(ent);
					tc->position = mEditorScene.camera().pos();
					tc->rotation = { 0.f, 0.f, 0.f, 1.f };
					LightComponent* lc = mgr.add_component<LightComponent>(ent);
					lc->color = { 1.f, 1.f, 1.f };
				}
			});
		});
		UI::window("Secondary window", [&]() {
			UI::container("Entity", [&]() {
				if (mActiveEntity == 0) {
					UI::label("No active ent");
					return;
				}

				if (UI::button("Delete")) {
					mEditorScene.manager().destroy(mActiveEntity);
					mActiveEntity = 0;
				}

				auto tf = mEditorScene.manager().get_component<TransformComponent>(mActiveEntity);
				if (tf) {
					UI::container("Transform", [&]() {
						UI::slider("x", -1000.f, 1000.f, tf->position.x);
						UI::slider("y", -1000.f, 1000.f, tf->position.y);
						UI::slider("z", -1000.f, 1000.f, tf->position.z);
					});
				}
			});
			UI::container("Entities", [&]() {
				mEditorScene.manager().view([&](u32 ent) {
					if (UI::button("Entity (" + std::to_string(ent) + ")")) {
						mActiveEntity = ent;
					}
				});
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
			for (u32 i = 0; i < mLogHistory.size(); i++) {
				auto& msg = mLogHistory[mLogHistory.size() - 1 - i];
				UI::label(msg);
			}
		});
		UI::window("Scene", [&]() {
			if (UI::button("Save")) {
				mEditorScene.save_scene("scene.scene");
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
