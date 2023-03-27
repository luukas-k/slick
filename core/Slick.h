#pragma once

#include "Core.h"

#include "utility/Logger.h"
#include "utility/Timer.h"
#include "utility/UUID.h"
#include "utility/FileWatcher.h"
#include "utility/CommandQueue.h"
#include "utility/ThreadPool.h"

#include "ecs/Manager.h"

#include "app/Application.h"
#include "app/Scene.h"
#include "app/ResourceManager.h"

#include "graphics/Surface.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Viewport.h"
#include "graphics/DebugRenderer.h"

#include "ui/UI.h"

#include "math/Mat.h"
#include "math/Vec.h"

#include "components/Transform.h"
#include "components/Renderable.h"
#include "components/Light.h"
#include "components/UUIDComponent.h"
#include "components/RigidBody.h"

#include "graphics/RenderSystem.h"

#include "network/Networking.h"

#include "physics/PhysicsSystem.h"

#include "audio/AudioDevice.h"