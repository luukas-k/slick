#pragma once

#include "Core.h"

#include "utility/Logger.h"

namespace Slick::ECS {

	constexpr const u32 MAX_COMPONENT_SIZE = 16;
	
	class Manager {
	public:
		Manager();
		~Manager();

		u32 create();
		void destroy(u32 eid);

		template<typename T>
		T* add_component(u32 eid) {
			mEntities[eid].components[type_id<T>()] = {};
			return get_component<T>(eid);
		}

		template<typename T>
		T* get_component(u32 eid) {
			if (mEntities[eid].components.contains(type_id<T>())) {
				return (T*)mEntities[eid].components[type_id<T>()].data;
			}
			return nullptr;
		}

		template<typename T>
		void remove_component(u32 eid) {
			mEntities[eid].components.erase(type_id<T>());
		}

		template<typename...T, typename FN>
		void view(FN&& fn) {
			for (auto& [eid, cdata] : mEntities) {
				std::tuple<T*...> t = get_data<T...>(cdata);
				if (is_valid<T...>(t)) {
					fn(eid, std::get<T*>(t)...);
				}
			}
		}

		inline u32 entity_count() const { return (u32)mEntities.size(); }

		template<typename...T, typename System>
		void register_system(System& system) {
			mSystems.push_back(SystemData{
				.data = &system,
				.on_update = [](void* ptr, Manager& mgr){ ((System*)ptr)->update(mgr); }
			});
		}

		inline void update_systems() {
			for (auto& sys : mSystems) {
				sys.on_update(sys.data, *this);
			}
		}
	private:
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
			void(*on_update)(void*, Manager&);
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
