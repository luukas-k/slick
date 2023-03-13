#include "Surface.h"
#pragma once

namespace Slick::Gfx {

	Input::Key glfw_key_to_key(int vk) {
		#define ENUM_KEY(key, name, glfw_kc) case glfw_kc: return Input::Key:: ## key;

		switch (vk) {
			__ENUM_KEYS
		}

		#undef ENUM_KEY

		return Input::Key::Unknown;
	}

	Input::Button glfw_button_to_button(int vk) {
		#define ENUM_BUTTON(key, name, glfw_kc) case glfw_kc: return Input::Button:: ## key;

		
		switch (vk) {
			__ENUM_BUTTONS
		}

		#undef ENUM_BUTTON

		return Input::Button::Unknown;
	}

	Surface::Surface() 
		:
		mHandle(nullptr),
		mOnKey([](Input::Key, bool){}),
		mOnButton([](Input::Button, bool){}),
		mOnCursorMove([](i32,i32){})
	{
		glfwInit();

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		
		mHandle = glfwCreateWindow(1280, 720, "title", nullptr, nullptr);
		glfwMakeContextCurrent(mHandle);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glfwSwapInterval(0);

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) {
			// Buffer detailed info: Buffer object 2 (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) 
			// will use VIDEO memory as the source for buffer object operations.
			if(id == 131185) 
				return;
			Utility::Assert(false, "[", id, "]: ", message);
		}, nullptr);
		
		glfwSetWindowUserPointer(mHandle, this);

		glfwSetKeyCallback(mHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			Surface* surf = (Surface*)glfwGetWindowUserPointer(window);
			if(action == GLFW_PRESS || action == GLFW_RELEASE)
				surf->mOnKey(glfw_key_to_key(key), action == GLFW_PRESS);
		});

		glfwSetMouseButtonCallback(mHandle, [](GLFWwindow* window, int btn, int action, int mods) {
			Surface* surf = (Surface*)glfwGetWindowUserPointer(window);
			if(action == GLFW_PRESS || action == GLFW_RELEASE)
				surf->mOnButton(glfw_button_to_button(btn), action == GLFW_PRESS);
		});

		glfwSetCursorPosCallback(mHandle, [](GLFWwindow* window, double cx, double cy){
			Surface* surf = (Surface*)glfwGetWindowUserPointer(window);
			surf->mOnCursorMove((i32)cx, (i32)cy);
		});

		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	Surface::~Surface() {
		glfwDestroyWindow(mHandle);
	}

	void Surface::update() {
		glfwPollEvents();
	}

	void Surface::present() {
		glfwSwapBuffers(mHandle);
	}

	i32 Surface::width() const {
		i32 w{};
		glfwGetFramebufferSize(mHandle, &w, nullptr);
		return w;
	}

	i32 Surface::height() const {
		i32 h{};
		glfwGetFramebufferSize(mHandle, nullptr, &h);
		return h;
	}

	bool Surface::should_close() const {
		return glfwWindowShouldClose(mHandle);
	}

	void Surface::set_key_callback(std::function<void(Input::Key, bool)> cb) {
		mOnKey = [old_cb = mOnKey, new_cb = cb](Input::Key kc, bool b) {
			new_cb(kc, b);
			old_cb(kc, b);
		};
	}

	void Surface::set_button_callback(std::function<void(Input::Button, bool)> cb) {
		mOnButton = [old_cb = mOnButton, new_cb = cb](Input::Button kc, bool b) {
			new_cb(kc, b);
			old_cb(kc, b);
		};
	}

	void Surface::set_cursor_move_callback(std::function<void(i32, i32)> cb) {
		mOnCursorMove = [old_cb = mOnCursorMove, new_cb = cb](i32 x, i32 y) {
			new_cb(x, y);
			old_cb(x, y);
		};
	}

}