#pragma once

#include "Core.h"

#include "utility/Logger.h"

#define __ENUM_KEYS	\
	ENUM_KEY(Key_0, "0", GLFW_KEY_0) \
	ENUM_KEY(Key_1, "1", GLFW_KEY_1) \
	ENUM_KEY(Key_2, "2", GLFW_KEY_2) \
	ENUM_KEY(Key_3, "3", GLFW_KEY_3) \
	ENUM_KEY(Key_4, "4", GLFW_KEY_4) \
	ENUM_KEY(Key_5, "5", GLFW_KEY_5) \
	ENUM_KEY(Key_6, "6", GLFW_KEY_6) \
	ENUM_KEY(Key_7, "7", GLFW_KEY_7) \
	ENUM_KEY(Key_8, "8", GLFW_KEY_8) \
	ENUM_KEY(Key_9, "9", GLFW_KEY_9) \
	ENUM_KEY(Key_A, "A", GLFW_KEY_A) \
	ENUM_KEY(Key_B, "B", GLFW_KEY_B) \
	ENUM_KEY(Key_C, "C", GLFW_KEY_C) \
	ENUM_KEY(Key_D, "D", GLFW_KEY_D) \
	ENUM_KEY(Key_E, "E", GLFW_KEY_E) \
	ENUM_KEY(Key_F, "F", GLFW_KEY_F) \
	ENUM_KEY(Key_G, "G", GLFW_KEY_G) \
	ENUM_KEY(Key_H, "H", GLFW_KEY_H) \
	ENUM_KEY(Key_I, "I", GLFW_KEY_I) \
	ENUM_KEY(Key_J, "J", GLFW_KEY_J) \
	ENUM_KEY(Key_K, "K", GLFW_KEY_K) \
	ENUM_KEY(Key_L, "L", GLFW_KEY_L) \
	ENUM_KEY(Key_M, "M", GLFW_KEY_M) \
	ENUM_KEY(Key_N, "N", GLFW_KEY_N) \
	ENUM_KEY(Key_O, "O", GLFW_KEY_O) \
	ENUM_KEY(Key_P, "P", GLFW_KEY_P) \
	ENUM_KEY(Key_Q, "Q", GLFW_KEY_Q) \
	ENUM_KEY(Key_R, "R", GLFW_KEY_R) \
	ENUM_KEY(Key_S, "S", GLFW_KEY_S) \
	ENUM_KEY(Key_T, "T", GLFW_KEY_T) \
	ENUM_KEY(Key_U, "U", GLFW_KEY_U) \
	ENUM_KEY(Key_V, "V", GLFW_KEY_V) \
	ENUM_KEY(Key_W, "W", GLFW_KEY_W) \
	ENUM_KEY(Key_X, "X", GLFW_KEY_X) \
	ENUM_KEY(Key_Y, "Y", GLFW_KEY_Y) \
	ENUM_KEY(Key_Z, "Z", GLFW_KEY_Z) \
	ENUM_KEY(Key_Space, "Space", GLFW_KEY_SPACE) \
	ENUM_KEY(Key_Shift, "Shift", GLFW_KEY_LEFT_SHIFT) \
	ENUM_KEY(Key_Ctrl, "Ctrl", GLFW_KEY_LEFT_CONTROL)

#define __ENUM_BUTTONS \
	ENUM_BUTTON(Button_Left, "Left mouse button", GLFW_MOUSE_BUTTON_LEFT) \
	ENUM_BUTTON(Button_Middle, "Middle mouse button", GLFW_MOUSE_BUTTON_MIDDLE) \
	ENUM_BUTTON(Button_Right, "Right mouse button", GLFW_MOUSE_BUTTON_RIGHT) 

namespace Slick::Input {

	enum struct Key : u32 {
		Unknown = 0,
		
	#define ENUM_KEY(kc, ...) kc,

		__ENUM_KEYS

	#undef ENUM_KEY

		Count
	};

	enum struct Button : u32 {
		Unknown = 0,

		#define ENUM_BUTTON(kc, ...) kc,

		__ENUM_BUTTONS

		#undef ENUM_BUTTON

		Count
	};

	class InputManager {
	public:
		InputManager();
		~InputManager();

		void update();

		void on_key(Key kc, bool state);
		void on_button(Button kc, bool state);
		inline void on_cursor_move(i32 nx, i32 ny) { mNewX = nx; mNewY = ny; }

		inline bool key_state(Key kc) const { return mKeys[(u32)kc].state; }
		inline bool button_state(Button kc) const { return mButtons[(u32)kc].state; }
		inline i32 cursor_x() const { return mCursorX; }
		inline i32 cursor_dx() const { return mCursorState == 2 ? (mCursorX - mOldCursorX) : 0; }
		inline i32 cursor_dy() const { return mCursorState == 2 ? (mCursorY - mOldCursorY) : 0; }
	private:
		struct Data {
			bool state;
			bool handled;
		};
		std::array<Data, (u32)Key::Count> mKeys;
		std::array<Data, (u32)Button::Count> mButtons;
		struct KeyEvent {
			Key kc;
			bool state;
			bool handled;
		};
		std::vector<KeyEvent> mKeyEvents;
		struct ButtonEvent {
			Button kc;
			bool state;
			bool handled;
		};
		std::vector<ButtonEvent> mButtonEvents;
		i32 mCursorState;
		i32 mOldCursorX, mOldCursorY;
		i32 mCursorX, mCursorY;
		i32 mNewX, mNewY;
	};

}

namespace Slick {

	template<>
	inline std::string format<Input::Key>(Input::Key kc) {
		#define ENUM_KEY(kc, name, ...) case Input::Key:: ## kc: return name;

		switch (kc) {
			__ENUM_KEYS
		}

		#undef ENUM_KEY

		return "";
	}

	template<>
	inline std::string format<Input::Button>(Input::Button kc) {
		#define ENUM_BUTTON(kc, name, ...) case Input::Button:: ## kc: return name;

		switch (kc) {
			__ENUM_BUTTONS
		}

		#undef ENUM_BUTTON

		return "";
	}
	
}