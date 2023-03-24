#pragma once

#include "Core.h"

namespace Slick {

	static constexpr u32 MAX_NAME_LENGTH = 64;

	struct NameComponent {
		char name[MAX_NAME_LENGTH];
	};

}