#include <fstream>
#include <cstdio>

#include <engine/serialization/archive.hpp>
#include <engine/math/math.hpp>

#include <engine/core/application.hpp>
#include <engine/game/scene.hpp>

#include "camera.hpp"
#include "renderable-mesh.hpp"
#include "util-components.hpp"

#include "game-render-context.hpp"

void renderMesh(Game&, float);
void renderSkybox(Game&, float);

class GameScene : public Scene {
	public:
		inline GameScene()
				: Scene() {
			addUpdateSystem(firstPersonCameraSystem);
			addUpdateSystem(updateCameraSystem);

			addRenderSystem(renderMesh);
			addRenderSystem(GameRenderContext::clear);
			addRenderSystem(GameRenderContext::flushStaticMeshes);
			addRenderSystem(GameRenderContext::applyLighting);
			addRenderSystem(renderSkybox);
			addRenderSystem(GameRenderContext::flush);
		}

		virtual void load(Game& game) override {
			GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

			struct IndexedModel::AllocationHints hints;
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(2);
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(3);
			hints.elementSizes.push_back(2 * 16);
			hints.instancedElementStartIndex = 4;

			game.getAssetManager().loadStaticMesh("cube", "./res/cube.obj", hints);
			game.getAssetManager().loadMaterial("bricks", "./res/bricks.dds",
					"./res/bricks-normal.dds", "./res/bricks-material.dds");
			game.getAssetManager().loadCubeMap("sargasso-diffuse", "./res/sargasso-diffuse.dds");
			game.getAssetManager().loadCubeMap("sargasso-specular", "./res/sargasso-specular.dds");
			game.getAssetManager().loadTexture("schlick-brdf", "./res/schlick-brdf.png");

			grc->setDiffuseIBL(game.getAssetManager().getCubeMap("sargasso-diffuse"));
			grc->setSpecularIBL(game.getAssetManager().getCubeMap("sargasso-specular"));
			grc->setBrdfLUT(game.getAssetManager().getTexture("schlick-brdf"));

			ECS::Entity e = game.getECS().create();
			game.getECS().assign<RenderableMesh>(e,
					&game.getAssetManager().getVertexArray("cube"),
					&game.getAssetManager().getMaterial("bricks"));
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
	Window window("My Window", 1200, 800);

	GameRenderContext renderContext(window.getWidth(), window.getHeight(),
			Math::perspective(Math::toRadians(70.f), (float)window.getWidth()
			/ (float)window.getHeight(), 0.1f, 1000.f));
	Game game(window, &renderContext,
			Memory::SharedPointer<Scene>(new GameScene()), false);

	Application::destroy();

	return 0;
}

void renderMesh(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, RenderableMesh>().each([&](
			TransformComponent& transform, RenderableMesh& mesh) {
		((GameRenderContext*)game.getRenderContext())->renderMesh(*mesh.vertexArray,
				*mesh.material, transform.transform);
	});
}

void renderSkybox(Game& game, float deltaTime) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

	Matrix4f mvp = Math::translate(grc->getCamera().viewProjection,
			Vector3f(grc->getCamera().view[3]));

	grc->getSkyboxCube().updateBuffer(1, &mvp, sizeof(Matrix4f));
	grc->renderSkybox(grc->getSpecularIBL(), grc->getLinearMipmapSampler());
}
