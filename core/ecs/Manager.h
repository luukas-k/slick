#pragma once

#include "Core.h"

#include "utility/Logger.h"

namespace Slick::ECS {

	constexpr const u32 MAX_COMPONENT_SIZE = 16;

	u32 gen_id();

	template<typename T>
	u32 type_id() {
		static u32 id = gen_id();
		return id;
	}

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
			return (T*)mEntities[eid].components[type_id<T>()].data;
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
					fn(std::get<T*>(t)...);
				}
			}
		}

		inline u32 entity_count() const { return (u32)mEntities.size(); }
	private:
		u32 mCount;
		struct Component {
			u8 data[MAX_COMPONENT_SIZE];
		};
		struct Components {
			std::unordered_map<u32, Component> components;
		};
		std::unordered_map<u32, Components> mEntities;

		template<typename T>
		T* get(Components& c) {
			return c.components.contains(type_id<T>()) ? (T*)c.components[type_id<T>()].data : nullptr;
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
