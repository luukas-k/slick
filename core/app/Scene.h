#pragma once

#include "Core.h"

#include "ecs/Manager.h"
#include "graphics/Camera.h"
#include "app/ResourceManager.h"
#include "components/Name.h"
#include "utility/Logger.h"

namespace Slick::App {

	class Scene {
	public:
		Scene(ResourceManager& resources);
		~Scene();

		void load_scene(const std::string& name);
		void save_scene(const std::string& name);

		void update(float dt);

		template<typename...Components, typename FN>
		inline void view(FN&& fn) {
			mManager.view<Components...>(fn);
		}

		template<typename...T>
		std::tuple<u32, T*...> create_entity(const std::string& name) {
			Utility::Assert(name.size() < MAX_NAME_LENGTH);

			u32 ent = mManager.create();
			(mManager.add_component<T>(ent), ...);
			NameComponent* nc = mManager.add_component<NameComponent>(ent);
			memset(nc->name, 0, MAX_NAME_LENGTH);
			memcpy(nc->name, name.c_str(), name.size() < MAX_NAME_LENGTH ? name.size() : MAX_NAME_LENGTH - 1);
			return {ent, mManager.get_component<T>(ent)...};
		}

		inline void destroy_entity(u32 eid) {
			mManager.destroy(eid);
		}

		template<typename System>
		void register_system_dynamic(System& system) {
			mManager.register_system_dynamic([this, sys = &system](ECS::Manager& mgr, float dt) {
				sys->update(*this, mgr, dt);
			});
		}

		template<typename System>
		void register_system_fixed(System& system) {
			mManager.register_system_fixed([this, sys = &system](ECS::Manager& mgr, float dt) {
				sys->fixed_update(*this, mgr, dt);
			});
		}

		template<typename Component>
		inline Component* get_component(u32 eid) {
			return mManager.get_component<Component>(eid);
		}

		std::string get_name(u32 eid) {
			auto nc = mManager.get_component<NameComponent>(eid);
			if(nc)
				return nc->name;
			return "[unknown]";
		}

		inline Gfx::Camera& camera() { return mCamera; }
		inline ResourceManager& resources() { return mResources; }
		// inline ECS::Manager& manager() { return mManager; }
	private:
		ResourceManager& mResources;
		ECS::Manager mManager;
		Gfx::Camera mCamera;
	};

}
