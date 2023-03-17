#pragma once

#include "Core.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "input/Input.h"

namespace Slick {
	namespace Gfx {

		class Surface {
		public:
			Surface();
			~Surface();

			void update();
			void present();

			i32 width() const;
			i32 height() const;

			bool should_close() const;

			void set_key_callback(std::function<void(Input::Key, bool)> cb);
			void set_button_callback(std::function<void(Input::Button, bool)> cb);
			void set_cursor_move_callback(std::function<void(i32, i32)> cb);
			void set_scroll_callback(std::function<void(i32, i32)> cb);
		private:
			GLFWwindow* mHandle;
			std::function<void(Input::Key, bool)> mOnKey;
			std::function<void(Input::Button, bool)> mOnButton;
			std::function<void(i32, i32)> mOnCursorMove;
			std::function<void(i32, i32)> mOnScroll;
		};

	}
}