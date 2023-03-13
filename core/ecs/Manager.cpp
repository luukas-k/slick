#include "Manager.h"

namespace Slick::ECS {
	
	u32 gen_id() {
		static u32 id = 1;
		return id++;
	}

	
	Manager::Manager() 
		:
		mCount(1)
	{}

	Manager::~Manager() {}

	u32 Manager::create() {
		u32 eid = mCount++;
		mEntities[eid] = {};
		return eid;
	}

	void Manager::destroy(u32 eid) {
		mEntities.erase(eid);
	}

}

