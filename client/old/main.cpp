//#include <engine/serialization/archive.hpp>

//#include <engine/core/application.hpp>
//#include <engine/ecs/ecs.hpp>

//#include <engine/game/game-render-context.hpp>
#include <engine/game/scene-manager.hpp>
//#include <engine/game/game-event-handler.hpp>

//#include <engine/physics/physics-engine.hpp>

//#include "game-scene.hpp"
//#include "game-scene-2.hpp"
#include "temp-scene.hpp"

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
	
	SceneManager::getInstance().load<TempScene>();
	
	/*Application::setResizeCallback([&](Window& win, uint32 width,
			uint32 height) {
		renderContext.resize(width, height);
	});*/

	return 0;
}

