#include "UI.h"
#include "UI.h"
#include "UI.h"

#include "utility/Logger.h"
#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "stb_truetype.h"

namespace Slick::UI {

	enum struct ElementType {
		Root,
		Window,
		Container,
		Button,
		Slider,
		TreeNode
	};

	enum struct ContainerLayout {
		Vertical,
		Horizontal
	};

}

namespace Slick {

	template<>
	std::string format<UI::ElementType>(UI::ElementType t) {
		switch (t) {
		case UI::ElementType::Button:		return "Button";
		case UI::ElementType::Container:	return "Container";
		case UI::ElementType::Root:			return "Root";
		}
		return "Unknown";
	}

	template<>
	std::string format<Gfx::Viewport>(Gfx::Viewport vp) {
		return "viewport{" + format(vp.x) + " " + format(vp.y) + " " + format(vp.w) + " " + format(vp.h) + "}";
	}

}

namespace Slick::UI {

	struct UIContainer {
		bool is_open;
		ContainerLayout layout;
		Math::fVec3 color;
	};

	struct UIButton {
		bool hovered;
		bool clicked;
	};

	struct UISlider {
		float min, max;
		float value;
	};

	struct UIWindow {
		i32 offset_x, offset_y;
		bool minimized;
	};

	struct UIElement {
		ElementType type;
		bool is_new;
		UIElement* parent;
		std::string label;
		std::vector<UIElement> children;
		Gfx::Viewport vp;

		UIWindow as_window;
		UIButton as_button;
		UIContainer as_container;
		UISlider as_slider;
	};

	struct UIContext {
		UIElement root;
		UIData data;
		UIElement* current;
		i32 screen_w, screen_h;
		Gfx::Renderer2D renderer;
		std::unordered_map<std::string, u32> textures;
		bool clicked, last_clicked;

		i32 drag_x, drag_y;
		i32 drag_cursor_x, drag_cursor_y;
		bool dragging;
	};

	UIContext* s_Context = nullptr;

	u32 get_texture(const std::string& name) {
		if (s_Context->textures.contains(name)) {
			return s_Context->textures[name];
		}

		i32 w{}, h{}, c{};
		stbi_set_flip_vertically_on_load(true);
		u8* data = stbi_load(name.c_str(), &w, &h, &c, 4);

		u32 id{ 0 };
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		s_Context->textures[name] = id;

		return id;
	}

	UIData* create_context() {
		s_Context = new UIContext{
			.root = UIElement{
				.type = ElementType::Root,
				.is_new = true,
				.parent = nullptr,
				.label = "root",
				.children = {},
				.vp = {0,0,0,0},
			},
			.data = {},
			.current = nullptr,
			.screen_w = 0, .screen_h = 0,
			.renderer = Gfx::Renderer2D()
		};

		return &s_Context->data;
	}

	UIData* get_ui_data() {
		return &s_Context->data;
	}

	UIElement* get_or_create(ElementType type, const std::string& label) {
		for (auto& e : s_Context->current->children) {
			if (e.type == type && e.label == label) {
				return &e;
			}
		}
		s_Context->current->children.push_back(UIElement{
			.type = type,
			.is_new = true,
			.parent = nullptr,
			.label = label,
			.children = {}
											   });
		return &s_Context->current->children[s_Context->current->children.size() - 1];
	}

	void set_current(UIElement* elem) {
		elem->parent = s_Context->current;
		s_Context->current = elem;
	}
	void set_current_as_parent() {
		s_Context->current = s_Context->current->parent;
	}

	void begin_frame() {
		s_Context->current = &s_Context->root;
	}

	void display_hierarchy(UIElement& e, u32 i) {
		Utility::Log(Utility::Repeat("\t", i), e.type, e.vp);

		switch (e.type) {
		case ElementType::Root:
		{
			for (auto& r : e.children) {
				display_hierarchy(r, i + 1);
			}
			break;
		}
		case ElementType::Container:
		{
			for (auto& r : e.children) {
				display_hierarchy(r, i + 1);
			}
			break;
		}
		case ElementType::Button:
		{
			break;
		}
		}
	}

	Gfx::Viewport calculate_size(UIElement& e) {
		switch (e.type) {
			case ElementType::Root:
			{
				for (auto& c : e.children) {
					return calculate_size(c);
				}
				return { 0, 0, 0, 0 };
			}
			case ElementType::Window:
			{
				if (e.as_container.layout == ContainerLayout::Horizontal) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						w += cw;
						if (ch > h)
							h = ch;
					}
					Gfx::Viewport content{ 0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10 };
					return content.grow(0, 0, 25, 0);
				}
				else if (e.as_container.layout == ContainerLayout::Vertical) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						h += ch;
						if (cw > w)
							w = cw;
					}
					Gfx::Viewport content{ 0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10 };
					return !e.as_window.minimized ? content.grow(0, 0, 25, 0) : Gfx::Viewport{0, 0, w, 25};
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
			}
			case ElementType::Container:
			{
				Gfx::Viewport content{};
				if (e.as_container.layout == ContainerLayout::Horizontal) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						w += cw;
						if (ch > h)
							h = ch;
					}
					content = { 0, 0, w + ((i32)e.children.size() - 1) * 5 + 10, h + 10 };
				}
				else if (e.as_container.layout == ContainerLayout::Vertical) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						h += ch;
						if (cw > w)
							w = cw;
					}
					content = { 0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10 };
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}

				return content.grow(0, 0, 25, 0);
			}
			case ElementType::Button:
			{
				return { 0, 0, (i32)(e.label.size() * 19), 25 };
			}
			case ElementType::Slider:
			{
				return { 0, 0, 100, 20 };
			}
		}
		Utility::Assert(false, "Unknown type.");
		return { 0, 0, 0, 0 };
	}

	void relayout(UIContext* ctx, UIElement& e, Gfx::Viewport vp) {
		switch (e.type) {
		case ElementType::Root:
		{
			e.vp = ctx->data.vp;
			ctx->screen_w = e.vp.w;
			ctx->screen_h = e.vp.h;

			Utility::Assert(e.children.size() == 1, "Root can only have a single child element.");
			for (auto& r : e.children) {
				relayout(ctx, r, e.vp);
			}
			return;
		}
		case ElementType::Window:
		{
			auto [cx, cy, ew, eh] = calculate_size(e);
			e.vp = Gfx::Viewport{ vp.x, vp.y + vp.h - eh, ew, eh }.offset(-e.as_window.offset_x, e.as_window.offset_y);
			// Utility::Log(e.as_window.offset_x, e.as_window.offset_y);

			Gfx::Viewport header = e.vp.top(25);
			Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0).shrink(5, 5, 5, 5);

			if(!e.as_window.minimized){
				if (e.as_container.layout == ContainerLayout::Vertical) {
					auto current = content;
					for (auto& r : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.top(ch));
						current = current.shrink(0, 0, ch + 5, 0);
					}
				}
				else if (e.as_container.layout == ContainerLayout::Horizontal) {
					auto current = content;
					for (auto& r : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.left(cw));
						current = current.shrink(cw + 5, 0, 0, 0);
					}
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
			}
			return;
		}
		case ElementType::TreeNode:
		case ElementType::Container:
		{
			auto [cx, cy, ew, eh] = calculate_size(e);

			e.vp = Gfx::Viewport{ vp.x, vp.y + vp.h - eh, ew, eh };
			Gfx::Viewport header = e.vp.top(25);
			Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0);

			if (e.as_container.layout == ContainerLayout::Vertical) {
				auto current = content.shrink(5, 5, 5, 5);
				for (auto& r : e.children) {
					auto [cx, cy, cw, ch] = calculate_size(r);
					relayout(ctx, r, current.top(ch));
					current = current.shrink(0, 0, ch + 5, 0);
				}
			}
			else if (e.as_container.layout == ContainerLayout::Horizontal) {
				auto current = content.shrink(5, 5, 5, 5);
				for (auto& r : e.children) {
					auto [cx, cy, cw, ch] = calculate_size(r);
					relayout(ctx, r, current.left(cw));
					current = current.shrink(cw + 5, 0, 0, 0);
				}
			}
			else {
				Utility::Assert(false, "Unknown layout.");
			}
			return;
		}
		case ElementType::Button:
		{
			auto [x, y, w, h] = calculate_size(e);
			e.vp = vp.left(w).top(h);
			return;
		}
		case ElementType::Slider:
		{
			auto [x, y, w, h] = calculate_size(e);
			e.vp = vp.left(w).top(h);
			return;
		}
		}

		Utility::Assert(false, "Unknown type.");
	}

	void render(UIContext* ctx, UIElement& e) {
		auto draw_vp = [&](Gfx::Viewport vp, Math::fVec3 color, i32 border_radius) {
			ctx->renderer.submit_rect(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				{ (float)(vp.x + vp.w) / ctx->data.vp.w, (float)(vp.y + vp.h) / ctx->data.vp.h },
				color,
				(float)border_radius / vp.w
			);
		};
		auto draw_tex = [&](Gfx::Viewport vp, const std::string& name, i32 border_radius) {
			ctx->renderer.submit_rect(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				{ (float)(vp.x + vp.w) / ctx->data.vp.w, (float)(vp.y + vp.h) / ctx->data.vp.h },
				{ 0.f, 0.f }, { 1.f, 1.f },
				get_texture("icon/" + name),
				(float)border_radius / vp.w
			);
		};
		auto draw_text = [&](Gfx::Viewport vp, const std::string& text) {
			ctx->renderer.submit_text(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				(float)vp.h / ctx->data.vp.h,
				text
			);
		};

		switch (e.type) {
			case ElementType::Root:
			{
				for (auto& c : e.children) {
					render(ctx, c);
				}
				return;
			}
			case ElementType::Window:
			{
				Gfx::Viewport header = e.vp.top(25);
				Gfx::Viewport close = header.right(25);
				Gfx::Viewport minimize = close.offset(-25, 0);
				Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0);

				if(!e.as_window.minimized)
					draw_vp(content, e.as_container.color, 5);
				draw_vp(header, { .2f, .2f, .2f }, 5);
				draw_vp(close.shrink(3, 3, 3, 3), { 1.f, 0.f, 0.f }, 10);
				draw_vp(minimize.shrink(3, 3, 3, 3), { 1.f, 1.f, 0.f }, 10);

				if (!e.as_window.minimized) {
					for (auto& c : e.children) {
						render(ctx, c);
					}
				}
				return;
			}
			case ElementType::Container:
			{
				Gfx::Viewport header = e.vp.top(25);
				Gfx::Viewport minimize = header.right(25);
				Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0);

				draw_vp(header, { 0.3f, 0.2f, 0.2f }, 10);
				draw_vp(content, e.as_container.color, 10);
				draw_vp(minimize.shrink(3, 3, 3, 3), { 1.f, 1.f, 0.f }, 10);
				for (auto& c : e.children) {
					render(ctx, c);
				}
				return;
			}
			case ElementType::Button:
			{
				Gfx::Viewport button_container = e.vp;
				Math::fVec3 color = e.as_button.hovered ? Math::fVec3{ 1.f, 0.f, 0.f } : Math::fVec3{ 0.5f, 0.5f, 0.5f };
				draw_vp(button_container, color, 5);
				draw_text(button_container.shrink(2,2,2,2), e.label);
				return;
			}
			case ElementType::Slider:
			{
				Gfx::Viewport slider_container = e.vp;
				float factor = (e.as_slider.value - e.as_slider.min) / (e.as_slider.max - e.as_slider.min);

				Gfx::Viewport slider_grabber = slider_container.shrink((i32)(80.f * factor), 0, 0, 0).left(20);

				draw_vp(slider_container, { 0.2f, 0.2f, 0.2f }, 5);
				draw_vp(slider_grabber, { 0.4f, 0.2f, 0.2f }, 5);
				return;
			}
		}

		Utility::Assert(false, "Unknown type.");
	}

	bool is_hovered(Gfx::Viewport vp) {
		return vp.contains(s_Context->data.cx, s_Context->data.vp.h - s_Context->data.cy);
	}

	void update(UIContext* ctx, UIElement& e) {
		switch (e.type) {
		case ElementType::Window:
		{
			auto& wnd = e.as_window;
			if (!ctx->dragging) {
				if (is_hovered(e.vp.top(25).shrink(0, 50, 0, 0)) && ctx->clicked) {
					ctx->drag_cursor_x = ctx->data.cx;
					ctx->drag_cursor_y = ctx->data.cy;
					ctx->drag_x = wnd.offset_x;
					ctx->drag_y = wnd.offset_y;
					ctx->dragging = true;
				}
				if (is_hovered(e.vp.top(25).right(25).offset(-25, 0)) && ctx->last_clicked && !ctx->clicked) {
					e.as_window.minimized = !e.as_window.minimized;
				}
			}
			else {
				i32 new_ox = ctx->drag_x + (ctx->drag_cursor_x - ctx->data.cx);
				i32 new_oy = ctx->drag_y + (ctx->drag_cursor_y - ctx->data.cy);

				wnd.offset_x = new_ox;
				wnd.offset_y = new_oy;

				if (!ctx->clicked) {
					ctx->dragging = false;
				}
			}
			break;
		}
		case ElementType::Button:
		{
			auto& btn = e.as_button;

			btn.hovered = is_hovered(e.vp);

			if (!btn.clicked) {
				btn.clicked = is_hovered(e.vp) && ctx->last_clicked && !ctx->clicked;
			}

			break;
		}
		}

		for (auto& c : e.children) {
			update(ctx, c);
		}
	}

	void end_frame() {
		Utility::Assert(s_Context->current == &s_Context->root);

		s_Context->renderer.on_resize(s_Context->data.vp);
		// display_hierarchy(s_Context->root, 0);
		relayout(s_Context, s_Context->root, {});

		s_Context->last_clicked = s_Context->clicked;
		s_Context->clicked = s_Context->data.clicked;
		update(s_Context, s_Context->root);

		s_Context->renderer.begin();
		render(s_Context, s_Context->root);
		s_Context->renderer.end();
	}

	bool button(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Button, label);
		if (elem->as_button.clicked) {
			elem->as_button.clicked = false;
			return true;
		}
		return false;
	}

	void slider(const std::string& label, float min, float max, float& v) {
		UIElement* elem = get_or_create(ElementType::Slider, label);
		elem->as_slider.min = min;
		elem->as_slider.max = max;
		elem->as_slider.value = v;
	}

	bool begin_tree(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::TreeNode, label);

		if (elem->is_new) {
			elem->as_container.is_open = true;
			elem->is_new = false;
			elem->as_container.color = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };
		}

		if (elem->as_container.is_open) {
			set_current(elem);
			return true;
		}
		return false;
	}

	void end_tree() {
		set_current_as_parent();
	}

	bool begin_container(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Container, label);

		if (elem->is_new) {
			elem->as_container.is_open = true;
			elem->is_new = false;
			elem->as_container.color = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };
		}

		if (elem->as_container.is_open) {
			set_current(elem);
			return true;
		}
		return false;
	}

	void end_container() {
		set_current_as_parent();
	}

	bool begin_window(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Window, label);

		if (elem->is_new) {
			elem->as_container.is_open = true;
			elem->as_container.color = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };
			elem->as_window.offset_x = 0;
			elem->as_window.offset_y = 0;

			elem->is_new = false;
		}

		if (elem->as_container.is_open) {
			set_current(elem);
			return true;
		}
		return false;
	}

	void end_window() {
		set_current_as_parent();
	}

}
