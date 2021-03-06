#include <engine/scene/scene-manager.hpp>

#include <engine/application/application.hpp>
#include <engine/rendering/render-context.hpp>
#include <engine/rendering/render-system.hpp>
#include <engine/resource/resource-manager.hpp>
#include <engine/physics/physics-engine.hpp>
#include <engine/ocean/ocean.hpp>

#include <engine/math/math.hpp>

#include "temp-scene.hpp"

int main() {
	Application::init("BlockSystems - Client", 800, 600);
	RenderContext::init();

	const int width = Application::ref().getWidth();
	const int height = Application::ref().getHeight();

	RenderSystem::init(RenderContext::ref(), width, height, Math::toRadians(70.f), 0.1f, 1000.f);

	ResourceManager::init();
	PhysicsEngine::init();


	SceneManager::init();

	// start the engine
	SceneManager::ref().load<TempScene>();

	SceneManager::destroy();

	Ocean::destroy();

	PhysicsEngine::destroy();
	ResourceManager::destroy();

	RenderSystem::destroy();
	RenderContext::destroy();
	Application::destroy();

	return 0;
}

