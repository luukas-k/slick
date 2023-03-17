#include "Application.h"

#include "utility/Timer.h"

namespace Slick::App{

	Application::Application() 
	{
		mSurface.set_key_callback([this](Input::Key kc, bool p) {
			for (auto& l : mLayers) {
				l.on_key(l.data, kc, p);
			}
		});
		mSurface.set_button_callback([this](Input::Button kc, bool p) {
			for (auto& l : mLayers) {
				l.on_button(l.data, kc, p);
			}
		});
		mSurface.set_cursor_move_callback([this](i32 x, i32 y) {
			for (auto& l : mLayers) {
				l.on_cursor_move(l.data, x, y);
			}
		});
		mSurface.set_scroll_callback([this](i32 x, i32 y) {
			for (auto& l : mLayers) {
				l.on_scroll(l.data, x, y);
			}
		});
	}
	Application::~Application() {
		for (auto& l : mLayers) {
			l.on_delete(l.data);
		}
	}

	void Application::run() {
		while (!mSurface.should_close()) {
			mSurface.update();
			for (auto& ld : mLayers) {
				ld.on_update(ld.data, this);
			}
			for (auto& ld : mLayers) {
				ld.on_render(ld.data, this, mSurface.width(), mSurface.height());
			}
			mSurface.present();
		}
	}

}