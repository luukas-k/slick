#pragma once

#include "Core.h"

namespace Slick::Gfx {

	struct Viewport {
		i32 x, y, w, h;
		inline Viewport shrink(i32 left, i32 right, i32 top, i32 bottom) { return Viewport{ x + left,  y + bottom, w - left - right, h - top - bottom }; }
		inline Viewport grow(i32 left, i32 right, i32 top, i32 bottom) { return shrink(-left, -right, -top, -bottom); }
		inline Viewport left(i32 n) { return Viewport{ x, y, n, h }; }
		inline Viewport right(i32 n) { return Viewport{ x + w - n, y, n, h }; }
		inline Viewport top(i32 n) { return Viewport{ x, y + h - n, w, n	}; }
		inline Viewport bottom(i32 n) { return Viewport{ x, y, w, n }; }
		inline Viewport offset(i32 dx, i32 dy) { return Viewport{ x + dx, y + dy, w, h };  }
		inline bool contains(i32 cx, i32 cy){ return (cx > x) && (cy > y) && (cx < (x + w)) && (cy < (y + h)); }
		inline bool contains(Viewport other) { 
			return 
				contains(other.x, other.y) && 
				contains(other.x + other.w, other.y) && 
				contains(other.x + other.w, other.y + other.h) && 
				contains(other.x, other.y + other.h); 
		}
	};

}