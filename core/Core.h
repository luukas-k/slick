#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <fstream>
#include <assert.h>
#include <array>
#include <mutex>
#include <thread>

namespace Slick {
	
	using f32 = float;
	using f64 = double;

	using i8  =  int8_t;
	using i16 =  int16_t;
	using i32 =  int32_t;
	using i64 =  int64_t;

	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	u32 gen_id();

	template<typename T>
	u32 type_id() {
		static u32 id = gen_id();
		return id;
	}

}