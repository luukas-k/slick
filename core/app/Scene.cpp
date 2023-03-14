#include "Scene.h"

namespace Slick::App {

	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	void Scene::update(float dt) {
		mManager.update_systems();
	}

}
