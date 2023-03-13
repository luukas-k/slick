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
		Button
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

namespace Slick::UI{

	struct UIContainer {
		bool is_open;
		ContainerLayout layout;
		Math::fVec3 color;
	};

	struct UIButton {
	};

	struct UIElement {
		ElementType type;
		bool is_new;
		UIElement* parent;
		std::string label;
		std::vector<UIElement> children;
		Gfx::Viewport vp;
		bool hovered, clicked, released, handled;
		
		UIContainer as_container;
	};

	struct UIContext {
		UIElement root;
		UIData data;
		UIElement* current;
		i32 screen_w, screen_h;
		Gfx::Renderer2D renderer;
	};

	UIContext* s_Context = nullptr;

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
				return {0, 0, 0,0};
			}
			case ElementType::Button: 
			{
				return {0, 0, (i32)e.label.size() * 25, 20};
			}
			case ElementType::Window:
			{
				if (e.as_container.layout == ContainerLayout::Horizontal) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(c);
						w += cw;
						if(ch > h)
							h = ch;
					}
					return {0, 0, w + ((i32)e.children.size() - 1) * 5 + 10, h + 10};
				}
				else if (e.as_container.layout == ContainerLayout::Vertical) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(c);
						h += ch;
						if(cw > w)
							w = cw;
					}
					Gfx::Viewport content{0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10};
					return content.grow(0, 0, 25, 0);
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
			}
			case ElementType::Container:
			{
				if (e.as_container.layout == ContainerLayout::Horizontal) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(c);
						w += cw;
						if(ch > h)
							h = ch;
					}
					return {0, 0, w + ((i32)e.children.size() - 1) * 5 + 10, h + 10};
				}
				else if (e.as_container.layout == ContainerLayout::Vertical) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(c);
						h += ch;
						if(cw > w)
							w = cw;
					}
					return {0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10};
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
			}
		}
		Utility::Assert(false, "Unknown type.");
		return {0, 0, 0, 0};
	}

	void relayout(UIContext* ctx, UIElement& e, Gfx::Viewport vp) {
		switch (e.type) {
			case ElementType::Root: 
			{
				if (ctx->data.vp.w == ctx->screen_w && ctx->data.vp.h && ctx->screen_h) {
					return;
				}
				e.vp = ctx->data.vp;
				ctx->screen_w = e.vp.w;
				ctx->screen_h = e.vp.h;

				Utility::Assert(e.children.size() == 1, "Root can only have a single child element.");
				for (auto& r : e.children) {
					relayout(ctx, r, e.vp);
				}
				break;
			}
			case ElementType::Window:
			{
				auto[cx, cy, ew, eh] = calculate_size(e);
				e.vp = Gfx::Viewport{vp.x, vp.y + vp.h - eh, ew, eh};

				Gfx::Viewport header = e.vp.top(25);
				Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0).shrink(5, 5, 5, 5);

				if (e.as_container.layout == ContainerLayout::Vertical) {
					auto current = content;
					for (auto& r : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.top(ch));
						current = current.shrink(0, 0, ch + 5, 0);
					}
				}
				else if (e.as_container.layout == ContainerLayout::Horizontal) {
					auto current = content;
					for (auto& r : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.left(cw));
						current = current.shrink(cw + 5, 0, 0, 0);
					}
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
				break;
			}
			case ElementType::Container: 
			{
				auto[cx, cy, ew, eh] = calculate_size(e);

				e.vp = Gfx::Viewport{vp.x, vp.y + vp.h - eh, ew, eh};
				Gfx::Viewport content = e.vp;

				if (e.as_container.layout == ContainerLayout::Vertical) {
					auto current = content.shrink(5, 5, 5, 5);
					for (auto& r : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.top(ch));
						current = current.shrink(0, 0, ch + 5, 0);
					}
				}
				else if (e.as_container.layout == ContainerLayout::Horizontal) {
					auto current = content.shrink(5, 5, 5, 5);
					for (auto& r : e.children) {
						auto[cx, cy, cw, ch] = calculate_size(r);
						relayout(ctx, r, current.left(cw));
						current = current.shrink(cw + 5, 0, 0, 0);
					}
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}
				break;
			}
			case ElementType::Button: 
			{
				auto[x, y, w, h] = calculate_size(e);
				e.vp = vp.left(w).top(h);
				break;
			}
		}
	}

	void render(UIContext* ctx, UIElement& e) {
		auto draw_vp = [&](Gfx::Viewport vp, Math::fVec3 color) {
			ctx->renderer.draw_rect(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				{ (float)(vp.x + vp.w) / ctx->data.vp.w, (float)(vp.y + vp.h) / ctx->data.vp.h },
				{0.f, 0.f}, {1.f, 1.f},
				color
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
				Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0);
				
				draw_vp(content, e.as_container.color);
				draw_vp(header, {.2f, .2f, .2f});
				draw_vp(close, {.6f, 0.f, 0.f});

				for (auto& c : e.children) {
					render(ctx, c);
				}
				return;
			}
			case ElementType::Container: 
			{
				Gfx::Viewport content = e.vp;
				draw_vp(content, e.as_container.color);
				for (auto& c : e.children) {
					render(ctx, c);
				}
				return;
			}
			case ElementType::Button: 
			{
				Gfx::Viewport button_container = e.vp;
				Math::fVec3 color = e.hovered ? Math::fVec3{1.f, 0.f, 0.f} : Math::fVec3{0.5f, 0.5f, 0.5f};
				draw_vp(button_container, color);
				draw_vp(button_container.shrink(2, 2, 2, 2), {0.f, 0.f, 0.f});
				draw_vp(button_container.shrink(4, 4, 4, 4), color);
				return;
			}
		}
	}

	void end_frame() {
		Utility::Assert(s_Context->current == &s_Context->root);

		s_Context->renderer.on_resize(s_Context->data.vp);
		// display_hierarchy(s_Context->root, 0);
		relayout(s_Context, s_Context->root, {});
		render(s_Context, s_Context->root);
	}

	bool is_hovered(Gfx::Viewport vp) {
		return vp.contains(s_Context->data.cx, s_Context->data.vp.h - s_Context->data.cy);
	}

	bool has_clicked() {
		return s_Context->data.clicked;
	}

	bool button(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Button, label);
		elem->hovered = is_hovered(elem->vp);
		
		if (elem->hovered && !elem->clicked) {
			elem->clicked = has_clicked();
			elem->handled = false;
		}

		if (elem->clicked && !elem->handled) {
			elem->handled = true;
			return true;
		}

		if(!has_clicked())
			elem->clicked = false;

		return false;
	}

	bool begin_container(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Container, label);
		elem->hovered = is_hovered(elem->vp);

		if (elem->is_new) {
			elem->as_container.is_open = true;
			elem->is_new = false;
			elem->as_container.color = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
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
		elem->hovered = is_hovered(elem->vp);

		if (elem->is_new) {
			elem->as_container.is_open = true;
			elem->is_new = false;
			elem->as_container.color = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
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
