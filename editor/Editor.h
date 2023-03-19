#pragma once

#include "Slick.h"

namespace Slick::Editor {

	class EditorLayer {
	public:
		EditorLayer();
		~EditorLayer();

		void update(App::Application& app);
		u32 mFrames{ 0 };
		void on_key(Input::Key kc, bool state);
		void on_button(Input::Button kc, bool state);
		void on_cursor_move(i32 x, i32 y);
		void on_scroll(i32, i32);
	private:
		App::Scene mEditorScene;
		Input::InputManager mInput;
		Utility::Timer mTimer;
		float mLastUpdate, mLastRender;
		float mFrameDelta;
		Net::Connection mConnection;
		u32 mActiveEntity;
		std::vector<std::string> mLogHistory;
		App::ResourceManager mResources;
		Gfx::RenderSystem mRenderer;
	};

	class ServerLayer {
	public:
		ServerLayer();
		~ServerLayer();

		// Layer
		void update(App::Application& app);
		void on_cursor_move(i32 w, i32 h);
		void on_key(Input::Key kc, bool state);
		void on_button(Input::Button kc, bool state);
		inline void on_scroll(i32, i32) {}

		// Server
		void start_server();
		void stop_server();
		bool is_active();
	private:
		App::Scene mScene;
		Net::Server mServer;
	};

}