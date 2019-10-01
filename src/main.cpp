#include <cstdio>

//#include <engine/serialization/archive.hpp>
#include <engine/math/math.hpp>

#include <engine/core/application.hpp>

#include "game-scene.hpp"
#include "game-render-context.hpp"

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

	Application::init();
	Window window("My Window", 1200, 800);

	GameRenderContext renderContext(window.getWidth(), window.getHeight(),
			Math::perspective(Math::toRadians(70.f), (float)window.getWidth()
			/ (float)window.getHeight(), 0.1f, 1000.f));
	Game game(window, &renderContext,
			Memory::SharedPointer<Scene>(new GameScene()), false);

	Application::destroy();

	return 0;
}

