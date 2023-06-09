#include "UI.h"

#include "utility/Logger.h"
#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "stb_truetype.h"

#include "nlohmann/json.hpp"

namespace Slick::UI {

	enum struct ElementType {
		Root,
		Window,
		Container,
		Button,
		Label,
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
			case UI::ElementType::Window:		return "Window";
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
		// Elements
		Gfx::Viewport vp_header;
		Gfx::Viewport vp_content;
		Gfx::Viewport vp_minimize;
		// Data
		bool is_open;
		ContainerLayout layout;
	};

	struct UIButton {
		bool hovered;
		bool clicked;
	};

	struct UISlider {
		// Elements
		Gfx::Viewport vp_container;
		Gfx::Viewport vp_grabber;

		// Data
		float min, max;
		float value, new_value;
		bool has_changed;
	};

	struct UIWindow {
		i32 offset_x, offset_y;
		i32 size_x, size_y;
		i32 scroll_x, scroll_y;
		bool dragging, minimized;
		i32 z_index;

		i32 drag_x, drag_y;
		i32 drag_cx, drag_cy;
	};

	struct UIElement {
		ElementType type;
		bool is_new;
		UIElement* parent;
		std::string label;
		intptr_t current_index;
		std::vector<UIElement> children;
		Gfx::Viewport vp;

		UIWindow as_window;
		UIButton as_button;
		UIContainer as_container;
		UISlider as_slider;
	};

	struct WindowCreateData {
		std::string name;
		i32 off_x, off_y;
		i32 z_index;
		bool minimized;
	};

	struct UIContext {
		UIElement root;
		UIData data;
		UIElement* current;
		i32 screen_w, screen_h;
		Gfx::Renderer2D renderer;
		std::unordered_map<std::string, u32> textures;
		bool clicked, last_clicked;
		bool consumed;

		std::vector<Gfx::Viewport> clamp_viewport;
		std::vector<WindowCreateData> create_window_data;
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
		
		using namespace nlohmann;
		std::fstream fs("ui.json", std::fstream::in);
		if (fs) {
			json data = json::parse(fs);

			for (auto& wnd : data["windows"]) {
				s_Context->create_window_data.push_back(WindowCreateData{
					.name = wnd["name"],
					.off_x = wnd["position_x"],
					.off_y = wnd["position_y"],
					.z_index = wnd["z_index"],
					.minimized = wnd["minimized"]
				});
			}
		}

		return &s_Context->data;
	}

	void destroy_ui() {
		using namespace nlohmann;

		json result;
		result["windows"] = json::array();

		for (auto& c : s_Context->root.children) {
			json wnd;
			
			wnd["name"] = c.label;
			wnd["position_x"] = c.as_window.offset_x;
			wnd["position_y"] = c.as_window.offset_y;
			wnd["z_index"] = c.as_window.z_index;
			wnd["minimized"] = c.as_window.minimized;

			result["windows"].push_back(wnd);
		}

		std::fstream fs("ui.json", std::fstream::out);
		if (!fs) {
			Utility::Log("Unable to open UI definition on exit.");
			return;
		}
		fs.clear(); // Erase old data
		std::string data = result.dump(4);
		fs.write(data.c_str(), data.size());
		fs.close();
	}

	UIData* get_ui_data() {
		return &s_Context->data;
	}

	UIElement* get_or_create(ElementType type, const std::string& label) {
		for (u32 i = s_Context->current->current_index; i < s_Context->current->children.size(); i++) {
			auto& e = s_Context->current->children[i];
			if (e.type == type && e.label == label) {
				u32 skipped = i - s_Context->current->current_index;
				if (skipped > 0) {
					// Remove skipped UI elements
					s_Context->current->children.erase(s_Context->current->children.begin() + s_Context->current->current_index, s_Context->current->children.begin() + s_Context->current->current_index + skipped);
				}
				s_Context->current->current_index++;
				e.current_index = 0;
				e.is_new = false;
				return &e;
			}
		}
		s_Context->current->children.insert(s_Context->current->children.begin() + s_Context->current->current_index, UIElement{
			.type = type,
			.is_new = true,
			.parent = nullptr,
			.label = label,
			.current_index = 0,
			.children = {},
		});
		return &s_Context->current->children[s_Context->current->current_index++];
	}

	void set_current(UIElement* elem) {
		elem->parent = s_Context->current;
		s_Context->current = elem;
	}
	void set_current_as_parent() {
		u32 left = s_Context->current->children.size() - s_Context->current->current_index;
		if (left > 0) {
			// Utility::Log("Deleted left overs n = ", left, ".");
			s_Context->current->children.erase(s_Context->current->children.begin() + s_Context->current->current_index, s_Context->current->children.begin() + s_Context->current->current_index + left);
		}
		s_Context->current = s_Context->current->parent;
	}

	void begin_frame() {
		s_Context->current = &s_Context->root;
		// Utility::Log(s_Context->current->current_index);
		s_Context->current->current_index = 0;
	}

	void display_hierarchy(UIElement& e, u32 i) {
		Utility::Log(Utility::Repeat("\t", i), e.type, "'" + e.label + "'", e.vp);
		for (auto& r : e.children) {
			display_hierarchy(r, i + 1);
		}
	}

	Gfx::Viewport calculate_size(UIElement& e) {
		switch (e.type) {
			case ElementType::Root:
			{
				return { 0, 0, 0, 0 };
			}
			case ElementType::Window:
			{
				Gfx::Viewport content{0,0,0,0};
				if (e.as_container.layout == ContainerLayout::Horizontal) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						w += cw;
						if (ch > h)
							h = ch;
					}
					if(w < 250)
						w = 250;
					content = { 0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10 };
					
				}
				else if (e.as_container.layout == ContainerLayout::Vertical) {
					i32 w = 0, h = 0;
					for (auto& c : e.children) {
						auto [cx, cy, cw, ch] = calculate_size(c);
						h += ch;
						if (cw > w)
							w = cw;
					}
					if(w < 250)
						w = 250;
					content = { 0, 0, w + 10, h + ((i32)e.children.size() - 1) * 5 + 10 };
				}
				else {
					Utility::Assert(false, "Unknown layout.");
				}

				// Clamp height to 400
				return !e.as_window.minimized ? content.top(content.h < 400 ? content.h + 25 : 400) : Gfx::Viewport{0, 0, content.w, 25};
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
				auto[tw, th] = s_Context->renderer.text_metrics(25.f / s_Context->data.vp.h, e.label);
				return { 0, 0, (i32)(tw * s_Context->data.vp.w) + 10 + 5, (i32)(th * s_Context->data.vp.h) };
			}
			case ElementType::Label:
			{
				auto[tw, th] = s_Context->renderer.text_metrics(25.f / s_Context->data.vp.h, e.label);
				return { 0, 0, (i32)(tw * s_Context->data.vp.w) + 10 + 5, (i32)(th * s_Context->data.vp.h) };
			}
			case ElementType::Slider:
			{
				return { 0, 0, 200, 20 };
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
			Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0).shrink(5, 5, 5, 5).offset(e.as_window.scroll_x, e.as_window.scroll_y);

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
			ew = ew < 100 ? 100 : ew;

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

			auto& cont = e.as_container;
			cont.vp_header = e.vp.top(25);
			cont.vp_minimize = header.right(25).shrink(3, 3, 3, 3);
			cont.vp_content = e.vp.shrink(0, 0, 25, 0);

			return;
		}
		case ElementType::Button:
		{
			auto [x, y, w, h] = calculate_size(e);
			e.vp = vp.left(w).top(h);

			return;
		}
		case ElementType::Label:
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
		auto clamp = [&](Gfx::Viewport vp) {
			ctx->clamp_viewport.push_back(vp);
		};
		auto unclamp = [&]() {
			ctx->clamp_viewport.pop_back();
		};
		auto get_clamp_vp = [&]() {
			if (ctx->clamp_viewport.size() > 0) {
				return ctx->clamp_viewport[ctx->clamp_viewport.size() - 1];
			}
			return Gfx::Viewport{-99999, -99999, 2 * 99999, 2 * 99999};
		};
		auto draw_vp = [&](Gfx::Viewport vp, Math::fVec3 color, i32 border_radius) {
			i32 top_y = Math::clamp<i32>(get_clamp_vp().y, get_clamp_vp().y + get_clamp_vp().h, vp.y + vp.h);
			i32 bottom_y = Math::clamp<i32>(get_clamp_vp().y, get_clamp_vp().y + get_clamp_vp().h, vp.y);

			ctx->renderer.submit_rect(
				{ (float)vp.x / ctx->data.vp.w, (float)bottom_y / ctx->data.vp.h },
				{ (float)(vp.x + vp.w) / ctx->data.vp.w, (float)(top_y) / ctx->data.vp.h },
				color,
				(float)border_radius // vp.w
			);
		};
		auto draw_tex = [&](Gfx::Viewport vp, const std::string& name, i32 border_radius) {
			if(!get_clamp_vp().contains(vp)) return;

			ctx->renderer.submit_rect(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				{ (float)(vp.x + vp.w) / ctx->data.vp.w, (float)(vp.y + vp.h) / ctx->data.vp.h },
				{ 0.f, 0.f }, { 1.f, 1.f },
				get_texture("icon/" + name),
				(float)border_radius // vp.w
			);
		};
		auto draw_text = [&](Gfx::Viewport vp, const std::string& text) {
			if(!get_clamp_vp().contains(vp.shrink(10, 10, 10, 10))) return;

			ctx->renderer.submit_text(
				{ (float)vp.x / ctx->data.vp.w, (float)vp.y / ctx->data.vp.h },
				(float)vp.h / ctx->data.vp.h,
				text
			);
		};

		switch (e.type) {
			case ElementType::Root:
			{
				std::vector<UIElement*> childs;
				for(auto& c : e.children) childs.push_back(&c);
				std::sort(childs.begin(), childs.end(), [](UIElement* a, UIElement* b) { return a->as_window.z_index > b->as_window.z_index; });

				for (auto& c : childs) {
					render(ctx, *c);
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
					draw_vp(content, { 0.1f, 0.1f, 0.1f }, 5);
				draw_vp(header, { .2f, .2f, .2f }, 5);
				draw_vp(close.shrink(3, 3, 3, 3), { 1.f, 0.f, 0.f }, 10);
				draw_vp(minimize.shrink(3, 3, 3, 3), { 1.f, 1.f, 0.f }, 10);
				draw_text(header.offset(5, 4), e.label);

				clamp(content);
				if (!e.as_window.minimized) {
					for (auto& c : e.children) {
						render(ctx, c);
					}
				}
				unclamp();
				return;
			}
			case ElementType::Container:
			{
				auto& cont = e.as_container;
				// Gfx::Viewport header = e.vp.top(25);
				// Gfx::Viewport minimize = header.right(25);
				// Gfx::Viewport content = e.vp.shrink(0, 0, 25, 0);

				draw_vp(cont.vp_header, { 0.3f, 0.3f, 0.3f }, 10);
				draw_vp(cont.vp_content, { 0.2f, 0.2f, 0.2f }, 10);
				draw_vp(cont.vp_minimize, { 1.f, 1.f, 0.f }, 10);
				draw_text(cont.vp_header.offset(5, 4), e.label);

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
				draw_text(button_container.offset(5,4), e.label);
				return;
			}
			case ElementType::Label:
			{
				Gfx::Viewport button_container = e.vp;
				draw_text(button_container.offset(5,4), e.label);
				return;
			}
			case ElementType::Slider:
			{
				auto& sldr = e.as_slider;
				
				draw_vp(sldr.vp_container, { 0.2f, 0.2f, 0.2f }, 5);
				draw_vp(sldr.vp_grabber, { 0.4f, 0.2f, 0.2f }, 5);
				draw_text(sldr.vp_container.offset(5, 5), format(e.as_slider.value));

				return;
			}
		}

		Utility::Assert(false, "Unknown type.");
	}

	bool is_hovered(Gfx::Viewport vp) {
		return vp.contains(s_Context->data.cx, s_Context->data.vp.h - s_Context->data.cy);
	}

	void set_window_topmost(UIContext* ctx, UIElement& e) {
		i32 min = 99999999;
		for (auto& s : e.parent->children) {
			if (s.as_window.z_index < min) {
				min = s.as_window.z_index;
			}
		}
		for (auto& s : e.parent->children) {
			s.as_window.z_index += 1;
		}
		e.as_window.z_index = min;
	}

	void update(UIContext* ctx, UIElement& e) {
		switch (e.type) {
			case ElementType::Root:
			{
				std::vector<UIElement*> childs;
				for(auto& c : e.children) childs.push_back(&c);
				// Sort top to bottom to update in order to update top to bottom order
				std::sort(childs.begin(), childs.end(), [](UIElement* a, UIElement* b) { return a->as_window.z_index < b->as_window.z_index; });

				for (auto& c : childs) {
					update(ctx, *c);
				}

				return;
			}
			case ElementType::Window:
			{
				auto& wnd = e.as_window;
				if (!wnd.dragging) {
					if (is_hovered(e.vp.top(25).shrink(0, 50, 0, 0)) && !ctx->last_clicked && ctx->clicked && !ctx->consumed) {
						ctx->consumed = true;
						wnd.drag_cx = ctx->data.cx;
						wnd.drag_cy = ctx->data.cy;
						wnd.drag_x = wnd.offset_x;
						wnd.drag_y = wnd.offset_y;
						wnd.dragging = true;
						set_window_topmost(ctx, e);
					}
					if (is_hovered(e.vp.top(25).right(25).offset(-25, 0)) && ctx->last_clicked && !ctx->clicked && !ctx->consumed) {
						ctx->consumed = true;
						e.as_window.minimized = !e.as_window.minimized;
						set_window_topmost(ctx, e);
					}
					if (is_hovered(e.vp.shrink(0, 0, 25, 0))) {
						e.as_window.scroll_x -= ctx->data.scroll_x * 25;
						e.as_window.scroll_y -= ctx->data.scroll_y * 25;
						ctx->data.scroll_x = 0;
						ctx->data.scroll_y = 0;
						if(e.as_window.scroll_x < 0)
							e.as_window.scroll_x = 0;
						if(e.as_window.scroll_y < 0)
							e.as_window.scroll_y = 0;
					}
				}
				else {
					i32 new_ox = wnd.drag_x + (wnd.drag_cx - ctx->data.cx);
					i32 new_oy = wnd.drag_y + (wnd.drag_cy - ctx->data.cy);

					wnd.offset_x = new_ox;
					wnd.offset_y = new_oy;

					if (!ctx->clicked) {
						wnd.dragging = false;
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
			case ElementType::Slider: 
			{
				auto& sldr = e.as_slider;
				
				float newVal = Math::clamp(sldr.min, sldr.max, sldr.min + ((float)(ctx->data.cx - 10 - e.vp.x)) / (e.vp.w - 20) * (sldr.max - sldr.min));
				if (is_hovered(e.vp) && ctx->clicked) {
					sldr.new_value = newVal;
					sldr.has_changed = true;
				}
				else {
					sldr.has_changed = false;
				}

				sldr.vp_container = e.vp;
				float factor = (e.as_slider.value - e.as_slider.min) / (e.as_slider.max - e.as_slider.min);

				sldr.vp_grabber = sldr.vp_container.shrink((i32)((e.vp.w - 20) * factor), 0, 0, 0).left(20);

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
		s_Context->consumed = false;
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

	void label(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::Label, label);
	}

	void slider(const std::string& label, float min, float max, float& v) {
		UIElement* elem = get_or_create(ElementType::Slider, label);

		elem->as_slider.value = v;
		if (elem->as_slider.has_changed) {
			v = elem->as_slider.new_value;
			elem->as_slider.value = v;
		}

		elem->as_slider.min = min;
		elem->as_slider.max = max;
	}

	bool begin_tree(const std::string& label) {
		UIElement* elem = get_or_create(ElementType::TreeNode, label);

		if (elem->is_new) {
			elem->as_container.is_open = true;
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
			elem->as_window.minimized = false;
			elem->as_container.is_open = true;

			elem->as_window.size_x = 300;
			elem->as_window.size_y = 300;
			elem->as_window.scroll_y = 0;
			elem->as_window.scroll_y = 0;
			elem->as_window.z_index = 0;

			for (auto& cwd : s_Context->create_window_data) {
				if (label == cwd.name) {
					elem->as_window.offset_x = cwd.off_x;
					elem->as_window.offset_y = cwd.off_y;
					elem->as_window.z_index = cwd.z_index;
					elem->as_window.minimized = cwd.minimized;
				}
			}
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
