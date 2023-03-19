#include "UUID.h"

#include <math.h>

namespace Slick::Utility{

	UUID generate_uuid() {
		return UUID{
			.data = {
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255),
				(u8)(rand() % 255)
			}
		};
	}

}
