#include "Manager.h"

Slick::u32 Slick::gen_id() {
	static u32 id = 1;
	return id++;
}

namespace Slick::ECS {
	
	Manager::Manager(float timestep) 
		:
		mCount(1),
		mCurrentTime(0.f),
		mLastUpdate(0.f),
		mInterval(timestep)
	{}

	Manager::~Manager() {}

	u32 Manager::create() {
		u32 eid = mCount++;
		mEntities[eid] = {};
		return eid;
	}

	void Manager::destroy(u32 eid) {
		mEntities.erase(eid);
		Utility::Log("Erased ", eid, " Left ", mEntities.size());
	}

}

