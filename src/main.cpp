#include <fstream>
#include <cstdio>

#include <engine/serialization/archive.hpp>
#include <engine/math/math.hpp>

#include <engine/core/application.hpp>
#include <engine/game/scene.hpp>

#include "camera.hpp"
#include "renderable-mesh.hpp"
#include "game-render-context.hpp"

#include <chrono>
#include <unordered_map>
#include <map>
#include <vector>
#include <tuple>

void renderMesh(Game&, float);
void updateCamera(Game&, float);

void moveCamera(Game&, float);

struct TransformComponent {
	Matrix4f transform;
};

struct CameraComponent {
	Camera* camera;
};

class GameScene : public Scene {
	public:
		inline GameScene()
				: Scene() {
			addUpdateSystem(moveCamera);
			addUpdateSystem(updateCamera);

			addRenderSystem(renderMesh);
		}

		virtual void load(Game& game) override {
			struct IndexedModel::AllocationHints hints;
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(2);
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(2 * 16);
			hints.instancedElementStartIndex = 4;

			game.getAssetManager().loadStaticMesh("cube", "./res/cube.obj", hints);
			game.getAssetManager().loadTexture("bricks", "./res/bricks.dds");

			ECS::Entity e = game.getECS().create();
			game.getECS().assign<RenderableMesh>(e,
					&game.getAssetManager().getVertexArray("cube"),
					&game.getAssetManager().getTexture("bricks"));
			game.getECS().assign<TransformComponent>(e,
					Math::translate(Matrix4f(1.f), Vector3f(0.f, 0.f, -5.f)));

			ECS::Entity cameraEntity = game.getECS().create();

			game.getECS().assign<TransformComponent>(cameraEntity, Matrix4f(1.f));
			game.getECS().assign<CameraComponent>(cameraEntity,
					&((GameRenderContext*)game.getRenderContext())->getCamera());

			DEBUG_LOG_TEMP2("Loaded");
		}

		virtual void unload(Game& game) override {
			DEBUG_LOG_TEMP2("Unloaded");
		}

		virtual ~GameScene() {
			DEBUG_LOG_TEMP2("Removed");
		}
	private:
};

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
	Window window("My Window", 800, 600);

	GameRenderContext renderContext(800, 600,
			Math::perspective(Math::toRadians(70.f), 4.f / 3.f, 0.1f, 1000.f));
	Game game(window, &renderContext,
			Memory::SharedPointer<Scene>(new GameScene()), false);

	Application::destroy();

	return 0;
}

void renderMesh(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, RenderableMesh>().each([&](
			TransformComponent& transform, RenderableMesh& mesh) {
		((GameRenderContext*)game.getRenderContext())->renderMesh(*mesh.vertexArray,
				*mesh.texture, transform.transform);
	});
}

void updateCamera(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& cc) {
		Camera& camera = *cc.camera;

		camera.view = transform.transform;
		camera.viewProjection = camera.projection * camera.view;

		camera.iView = Math::inverse(camera.view);
		camera.iViewProjection = Math::inverse(camera.viewProjection);
	});
}

void moveCamera(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& cc) {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (Application::isKeyDown(Input::KEY_W)) {
			z += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_S)) {
			z -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_A)) {
			x += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_D)) {
			x -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_Q)) {
			y += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_E)) {
			y -= 1.f;
		}

		transform.transform = Math::translate(transform.transform,
				Vector3f(x, y, z) * (5.f * deltaTime));
	});
}
