#include "Scene.h"

#include "nlohmann/json.hpp"

#include "components/Transform.h"
#include "components/Renderable.h"
#include "components/Light.h"
#include "components/UUIDComponent.h"

namespace Slick::App {

	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	void Scene::load_scene(const std::string& name) {
		using namespace nlohmann;

		
	}

	template<typename T>
	nlohmann::json serialize_component(T* ent) {
		return {};
	}

	template<>
	nlohmann::json serialize_component<TransformComponent>(TransformComponent* tc) {
		nlohmann::json res;
		res["pos"][0] = tc->position.x;
		res["pos"][1] = tc->position.y;
		res["pos"][2] = tc->position.z;
		res["rot"][0] = tc->rotation.x;
		res["rot"][1] = tc->rotation.y;
		res["rot"][2] = tc->rotation.z;
		return res;
	}

	template<>
	nlohmann::json serialize_component<LightComponent>(LightComponent* lc) {
		nlohmann::json res;
		res["color"][0] = lc->color.x;
		res["color"][1] = lc->color.y;
		res["color"][2] = lc->color.z;
		return res;
	}

	template<>
	nlohmann::json serialize_component<RenderableComponent>(RenderableComponent* rc) {
		nlohmann::json res;
		res["material"] = "mat." + std::to_string(rc->material);
		res["mesh"] = "mesh." + std::to_string(rc->mesh);
		return res;
	}

	template<typename T>
	void serialize_component_if_exists(nlohmann::json& data, const std::string& name, ECS::Manager& mgr, u32 ent) {
		if (mgr.get_component<T>(ent)) {
			data[name] = serialize_component<T>(mgr.get_component<T>(ent));
		}
	}

	void Scene::save_scene(const std::string& name) {
		using namespace nlohmann;

		json scene;
		scene["entities"] = json::array();

		mManager.view<UUIDComponent>([&](u32 eid, UUIDComponent* uuid) {
			json ent;

			ent["id"] = format(uuid->uuid);
			
			serialize_component_if_exists<TransformComponent>(ent, "TransformComponent", mManager, eid);
			serialize_component_if_exists<RenderableComponent>(ent, "RenderableComponent", mManager, eid);
			serialize_component_if_exists<LightComponent>(ent, "LightComponent", mManager, eid);

			scene["entities"].push_back(ent);
		});

		std::fstream fs(name, std::fstream::out);
		if (!fs) {
			Utility::Log("Unable to save scene.");
			return;
		}
		auto fdata = scene.dump(4);
		fs.write(fdata.c_str(), fdata.size());
		fs.close();
	}

	void Scene::update(float dt) {
		mManager.update_systems();
	}

}
