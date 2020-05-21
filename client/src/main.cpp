#include <engine/scene/scene-manager.hpp>

#include <engine/application/application.hpp>
#include <engine/rendering/render-context.hpp>
#include <engine/rendering/render-system.hpp>

#include <engine/math/math.hpp>

#include "temp-scene.hpp"

int main() {
	Application::init("BlockSystems - Client", 800, 600);
	RenderContext::init();

	const int width = Application::ref().getWidth();
	const int height = Application::ref().getHeight();

	RenderSystem::init(RenderContext::ref(), width, height, Math::toRadians(70.f), 0.1f, 100.f);

	SceneManager::ref().load<TempScene>();

	return 0;
}

