#pragma once

#include "Core.h"

#include "utility/Logger.h"

namespace Slick::ECS {

	constexpr const u32 MAX_COMPONENT_SIZE = 32;
	
	class Manager {
	public:
		Manager(float timestep);
		~Manager();

		u32 create();
		void destroy(u32 eid);

		template<typename T>
		T* add_component(u32 eid) {
			static_assert(sizeof(T) <= MAX_COMPONENT_SIZE);
			Utility::Assert(mEntities.contains(eid));
			mEntities[eid].components[type_id<T>()] = {};
			return get_component<T>(eid);
		}

		template<typename T>
		T* get_component(u32 eid) {
			if (mEntities.contains(eid) && mEntities[eid].components.contains(type_id<T>())) {
				return (T*)mEntities[eid].components[type_id<T>()].data;
			}
			return nullptr;
		}

		template<typename T>
		void remove_component(u32 eid) {
			if (mEntities.contains(eid)) {
				mEntities[eid].components.erase(type_id<T>());
			}
		}

		template<typename...T, typename FN>
		void view(FN&& fn) {
			for (auto& [eid, cdata] : mEntities) {
				if(eid == 0) continue;
				std::tuple<T*...> t = get_data<T...>(cdata);
				if (is_valid<T...>(t)) {
					fn(eid, std::get<T*>(t)...);
				}
			}
		}

		inline u32 entity_count() const { return (u32)mEntities.size(); }

		template<typename...T, typename System>
		void register_system_fixed(System& system) {
			mSystems.push_back(SystemData{
				.data = &system,
				.fixed_update = true,
				.on_update = [](void* ptr, Manager& mgr, float dt){ ((System*)ptr)->fixed_update(mgr, dt); }
			});
		}

		template<typename...T, typename System>
		void register_system_dynamic(System& system) {
			mSystems.push_back(SystemData{
				.data = &system,
				.fixed_update = false,
				.on_update = [](void* ptr, Manager& mgr, float dt){ ((System*)ptr)->update(mgr, dt); }
			});
		}

		inline void update_systems(float dt) {
			mCurrentTime += dt;
			
			while (mLastUpdate + mInterval < mCurrentTime) {
				mLastUpdate += mInterval;
				for (auto& sys : mSystems) {
					if(sys.fixed_update)
						sys.on_update(sys.data, *this, mInterval);
				}
			}

			for (auto& sys : mSystems) {
				if(!sys.fixed_update)
					sys.on_update(sys.data, *this, dt);
			}
		}
	private:
		float mLastUpdate, mCurrentTime, mInterval;
		u32 mCount;
		struct Component {
			u8 data[MAX_COMPONENT_SIZE];
		};
		struct Components {
			std::unordered_map<u32, Component> components;
		};
		std::unordered_map<u32, Components> mEntities;
		struct SystemData {
			void* data;
			bool fixed_update;
			void(*on_update)(void*, Manager&, float);
		};
		std::vector<SystemData> mSystems;

		template<typename T>
		T* get(Components& c) {
			if (c.components.contains(type_id<T>())) {
				return (T*)c.components[type_id<T>()].data;
			}
			return nullptr;
		}

		template<typename...T>
		std::tuple<T*...> get_data(Components& cdata) {
			std::tuple<T*...> d{get<T>(cdata)...};
			return d;
		}

		template<typename...T>
		bool is_valid(const std::tuple<T*...>& d) {
			std::vector<bool> k{ (std::get<T*>(d) == nullptr)... };
			for(auto q : k) 
				if(q) return false;
			return true;
		}
	};

}
