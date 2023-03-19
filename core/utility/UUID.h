#pragma once

#include "Core.h"
#include "Logger.h"

namespace Slick::Utility {

	struct UUID {
		u8 data[16];
	};

	UUID generate_uuid();

}

namespace Slick {

	template<>
	inline std::string format<Utility::UUID>(Utility::UUID v) {
		auto hex = [](u8 v) {
			char map[]{
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
			};
			std::string res;
			res.push_back(map[(v >> 4) & 0xF]);
			res.push_back(map[v & 0xF]);
			return res;
		};
		return
			hex(v.data[0]) + hex(v.data[1]) + hex(v.data[2]) + hex(v.data[3]) + "-" +
			hex(v.data[4]) + hex(v.data[5]) + "-" + hex(v.data[6]) + hex(v.data[7]) + "-" +
			hex(v.data[8]) + hex(v.data[9]) + "-" + hex(v.data[10]) + hex(v.data[11]) + "-" +
			hex(v.data[12]) + hex(v.data[13]) + hex(v.data[14]) + hex(v.data[15]);
	}

}