#pragma once

#include "Core.h"

#include "graphics/Renderer2D.h"

namespace Slick::UI {

	struct UIData {
		Gfx::Viewport vp;
		i32 cx, cy;
		bool clicked;
		i32 scroll_x, scroll_y;
	};

	UIData* create_context();
	void destroy_ui();

	UIData* get_ui_data();
	
	void begin_frame();
	void end_frame();

	bool button(const std::string& label);
	void label(const std::string& label);
	void slider(const std::string& label, float min, float max, float& v);

	bool begin_tree(const std::string& label);
	void end_tree();

	bool begin_container(const std::string& label);
	void end_container();
	
	bool begin_window(const std::string& label);
	void end_window();

	template<typename FN>
	void frame(FN&& fn) {
		begin_frame();
		fn();
		end_frame();
	}

	template<typename FN>
	void container(const std::string& label, FN&& fn) {
		if (begin_container(label)) {
			fn();
			end_container();
		}
	}

	template<typename FN>
	void window(const std::string& label, FN&& fn) {
		if (begin_window(label)) {
			fn();
			end_window();
		}
	}

	template<typename FN>
	void tree(const std::string& label, FN&& fn) {
		if (begin_tree(label)) {
			fn();
			end_tree();
		}
	}

}