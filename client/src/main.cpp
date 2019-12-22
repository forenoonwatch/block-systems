#include <cstdio>

//#include <engine/serialization/archive.hpp>
#include <engine/math/math.hpp>

#include <engine/core/application.hpp>

//#include "game-scene.hpp"
#include "game-scene-2.hpp"

#include <engine/game/game-render-context.hpp>

int main() {
	//std::ifstream inFile("entt.cereal", std::ios::binary);
	//InputArchive archive(inFile);

	//registry.loader()
	//		.entities(archive)
	//		.component<Camera>(archive);

	//registry.view<Camera>().each([](Camera& camera) {
	//	const Matrix4f& m = camera.projection;
	//	printf("%.2f, %.2f, %.2f, %.2f\n", m[0][0], m[1][1], m[2][2], m[3][3]);
	//});

	Application app;
	Window* window = app.createWindow("My Window", 1200, 800);
	window->moveToCenter();

	Memory::SharedPointer<Scene> scene =
			//Memory::SharedPointer<Scene>(new GameScene());
			Memory::SharedPointer<Scene>(new GameScene2());

	GameRenderContext renderContext(window->getWidth(), window->getHeight(),
			Math::toRadians(70.f), 0.1f, 1000.f);
	Game game(app, *window, &renderContext, false);

	/*Application::setResizeCallback([&](Window& win, uint32 width,
			uint32 height) {
		renderContext.resize(width, height);
	});*/

	game.loadScene(scene);

	return 0;
}

