#pragma once

#include "Core.h"

#include "ecs/Manager.h"
#include "graphics/Camera.h"
#include "app/ResourceManager.h"

namespace Slick::App {

	class Scene {
	public:
		Scene(ResourceManager& resources);
		~Scene();

		void load_scene(const std::string& name);
		void save_scene(const std::string& name);

		void update(float dt);

		inline ECS::Manager& manager() { return mManager; }

		template<typename...T>
		std::tuple<u32, T*...> create_entity() {
			u32 ent = mManager.create();
			(mManager.add_component<T>(ent), ...);
			return {ent, mManager.get_component<T>(ent)...};
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

		inline Gfx::Camera& camera() { return mCamera; }
		inline ResourceManager& resources() { return mResources; }
	private:
		ResourceManager& mResources;
		ECS::Manager mManager;
		Gfx::Camera mCamera;
	};

}
