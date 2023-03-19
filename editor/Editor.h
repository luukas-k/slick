#pragma once

#include "Slick.h"

namespace Slick::Editor {

	enum struct Format : u8 {
		Unknown,

		Float4,
		Float3,
		Float2,
		Float1,
		UInt16,
	};

	struct BufferReference {
		u32 buffer, offset;
		Format fmt;
	};

	struct Mesh {
		/*i32 posBuffer, posOffset;
		Format posFormat;*/
		BufferReference position;
		
		BufferReference normal;
		bool hasNormals;

		BufferReference tangent;
		bool hasTangents;

		BufferReference uvs;
		bool hasUvs;

		BufferReference indices;
		bool hasIndices;

		u32 drawCount;
	};

	struct Material {
		Math::fVec3 baseColor;
		i32 baseColorTexture;

		float metallic, roughness;
		i32 metallicRoughnessTexture;

		i32 normalTexture;
	};

	class EditorLayer {
	public:
		EditorLayer();
		~EditorLayer();

		void update(App::Application& app);
		u32 mFrames{ 0 };
		void fixed_update(float dt);
		void render(App::Application& app, i32 w, i32 h);
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
		u32 vao{};
		std::vector<Mesh> mMeshes;
		std::vector<Material> mMaterials;
		Gfx::Shader mProgram;
		Net::Connection mConnection;
		u32 mActiveEntity;
		std::vector<std::string> mLogHistory;
	};

	class ServerLayer {
	public:
		ServerLayer();
		~ServerLayer();

		// Layer
		void update(App::Application& app);
		void render(App::Application& app, i32 w, i32 h);
		void on_cursor_move(i32 w, i32 h);
		void on_key(Input::Key kc, bool state);
		void on_button(Input::Button kc, bool state);
		inline void on_scroll(i32, i32){}

		// Server
		void start_server();
		void stop_server();
		bool is_active();
	private:
		App::Scene mScene;
		Net::Server mServer;
	};

}