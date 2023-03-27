#pragma once

#include "Slick.h"

namespace Slick::Editor {

	class EditorLayer {
	public:
		EditorLayer();
		~EditorLayer();

		void update(App::Application& app);
		void on_key(Input::Key kc, bool state);
		void on_button(Input::Button kc, bool state);
		void on_cursor_move(i32 x, i32 y);
		void on_scroll(i32, i32);

		void load_to_scene(const std::string& fname, Math::fVec3 pos);

		inline App::Scene& active_scene() { return mEditorScene; }
	private:
		App::ResourceManager mResources;
		App::Scene mEditorScene;
		Input::InputManager mInput;
		Utility::Timer mTimer;
		u32 mFrames{ 0 };
		float mSensitivity;
		Net::Connection mConnection;
		u32 mActiveEntity;
		std::vector<std::string> mLogHistory;
		Gfx::RenderSystem mRenderer;
		Gfx::DebugRenderer mDebugRenderer;
		Physics::PhysicsSystem mPhysics;
		Utility::ThreadPool mPool;
		Utility::CommandQueue mQueue;
		Audio::AudioDevice mAudio;
		bool mShowUI;

		bool mTranslating;
		Gfx::SelectedAxis mTranslationAxis;
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
		App::ResourceManager mResources;
		App::Scene mScene;
		Net::Server mServer;
	};

}