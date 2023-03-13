#pragma once

#include "Core.h"

#include "graphics/Surface.h"

namespace Slick {
	namespace App {

		class Application {
		public:
			Application();
			~Application();

			void run();

			template<typename T>
			void add_layer(const std::string& name, T* value) {
				mLayers.push_back(LayerInfo{
					name,
					(void*)value,
					[](void* ptr){ ((T*)ptr)->update(); },
					[](void* ptr, i32 w, i32 h){ ((T*)ptr)->render(w, h); },
					[](void* ptr, Input::Key kc, bool b) { ((T*)ptr)->on_key(kc, b); },
					[](void* ptr, Input::Button kc, bool b) { ((T*)ptr)->on_button(kc, b); },
					[](void* ptr, i32 x, i32 y){ ((T*)ptr)->on_cursor_move(x, y); }
				});
			}
		private:
			struct LayerInfo {
				std::string name;
				void* data;
				void(*on_update)(void*);
				void(*on_render)(void*, i32, i32);
				void(*on_key)(void*, Input::Key, bool);
				void(*on_button)(void*, Input::Button, bool);
				void(*on_cursor_move)(void*, i32, i32);
			};
			Gfx::Surface mSurface;
			std::vector<LayerInfo> mLayers;
		};

	}
}