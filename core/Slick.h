#pragma once

#include "Core.h"

#include "utility/Logger.h"
#include "utility/Timer.h"
#include "utility/UUID.h"
#include "utility/FileWatcher.h"

#include "ecs/Manager.h"

#include "app/Application.h"
#include "app/Scene.h"
#include "app/ResourceManager.h"

#include "graphics/Surface.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Viewport.h"

#include "ui/UI.h"

#include "math/Mat.h"
#include "math/Vec.h"

#include "ecs/components/Transform.h"
#include "ecs/components/Renderable.h"
#include "ecs/components/Light.h"
#include "ecs/components/UUIDComponent.h"

#include "ecs/systems/Renderer.h"

#include "network/Networking.h"