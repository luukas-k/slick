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
			void add_layer(const std::string& name, T* value, bool should_delete = false) {
				mLayers.push_back(LayerInfo{
					name,
					type_id<T>(),
					(void*)value,
					should_delete ? [](void* ptr){ delete (T*)ptr; } : [](void*){},
					[](void* ptr, Application* app){ ((T*)ptr)->update(*app); },
					[](void* ptr, Input::Key kc, bool b) { ((T*)ptr)->on_key(kc, b); },
					[](void* ptr, Input::Button kc, bool b) { ((T*)ptr)->on_button(kc, b); },
					[](void* ptr, i32 x, i32 y){ ((T*)ptr)->on_cursor_move(x, y); },
					[](void* ptr, i32 x, i32 y){ ((T*)ptr)->on_scroll(x, y); },
				});
			}

			template<typename T, typename...K>
			T* create_layer(const std::string& name, K...args) {
				T* layer = new T(std::forward<K>(args)...);
				add_layer(name, layer, true);
				return layer;
			}

			template<typename T>
			T* get_layer() {
				for (auto& l : mLayers) {
					if (l.type == type_id<T>()) {
						return (T*)l.data;
					}
				}
				return nullptr;
			}

			inline Gfx::Surface& surface() { return mSurface; }

		private:
			struct LayerInfo {
				std::string name;
				u32 type;
				void* data;
				void(*on_delete)(void*);
				void(*on_update)(void*, Application* app);
				void(*on_key)(void*, Input::Key, bool);
				void(*on_button)(void*, Input::Button, bool);
				void(*on_cursor_move)(void*, i32, i32);
				void(*on_scroll)(void*, i32, i32);
			};
			Gfx::Surface mSurface;
			std::vector<LayerInfo> mLayers;
		};

	}
}