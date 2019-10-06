#include "game-scene.hpp"

#include "camera.hpp"
#include "renderable-mesh.hpp"
#include "util-components.hpp"

#include "game-render-context.hpp"

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

static void renderMesh(Game&, float);
static void renderSkybox(Game&, float);
static void toggleFullscreenSystem(Game&, float);

GameScene::GameScene()
		: Scene() {
	addUpdateSystem(::firstPersonCameraSystem);
	addUpdateSystem(::updateCameraSystem);
	addUpdateSystem(::toggleFullscreenSystem);
	addUpdateSystem(::updateShipBuildInfo);

	addRenderSystem(::renderMesh);
	addRenderSystem(::shipRenderSystem);

	addRenderSystem(GameRenderContext::clear);
	addRenderSystem(GameRenderContext::flushStaticMeshes);
	addRenderSystem(GameRenderContext::applyLighting);
	addRenderSystem(::renderSkybox);
	addRenderSystem(GameRenderContext::flush);
}

void GameScene::load(Game& game) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

	struct IndexedModel::AllocationHints hints;
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(2);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(2 * 16);
	hints.instancedElementStartIndex = 4;

	game.getAssetManager().loadStaticMesh("cube", "cube", "./res/cube.obj", hints);
	game.getAssetManager().loadStaticMesh("tetrahedron", "tetrahedron",
			"./res/tetrahedron.obj", hints);
	game.getAssetManager().loadStaticMesh("wedge", "wedge", "./res/wedge.obj", hints);
	game.getAssetManager().loadStaticMesh("five-sixths-block", "five-sixths-block",
			"./res/five-sixths-block.obj", hints);
	
	game.getAssetManager().loadMaterial("bricks", "./res/bricks.dds",
			"./res/bricks-normal.dds", "./res/bricks-material.dds");
	game.getAssetManager().loadCubeMap("sargasso-diffuse", "./res/sargasso-diffuse.dds");
	game.getAssetManager().loadCubeMap("sargasso-specular", "./res/sargasso-specular.dds");
	game.getAssetManager().loadTexture("schlick-brdf", "./res/schlick-brdf.png");

	grc->setDiffuseIBL(game.getAssetManager().getCubeMap("sargasso-diffuse"));
	grc->setSpecularIBL(game.getAssetManager().getCubeMap("sargasso-specular"));
	grc->setBrdfLUT(game.getAssetManager().getTexture("schlick-brdf"));

	BlockInfo::registerType(BlockInfo::TYPE_BASIC_CUBE, Vector3i(1, 1, 1),
			game.getAssetManager().getModel("cube"),
			game.getAssetManager().getVertexArray("cube"),
			game.getAssetManager().getMaterial("bricks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_TETRA, Vector3i(1, 1, 1),
			game.getAssetManager().getModel("tetrahedron"),
			game.getAssetManager().getVertexArray("tetrahedron"),
			game.getAssetManager().getMaterial("bricks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE, Vector3i(1, 1, 1),
			game.getAssetManager().getModel("wedge"),
			game.getAssetManager().getVertexArray("wedge"),
			game.getAssetManager().getMaterial("bricks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_FIVE_SIXTH, Vector3i(1, 1, 1),
			game.getAssetManager().getModel("five-sixths-block"),
			game.getAssetManager().getVertexArray("five-sixths-block"),
			game.getAssetManager().getMaterial("bricks"));

	/*ECS::Entity e = game.getECS().create();
	game.getECS().assign<RenderableMesh>(e,
			&game.getAssetManager().getVertexArray("cube"),
			&game.getAssetManager().getMaterial("bricks"));
	game.getECS().assign<TransformComponent>(e,
			Math::translate(Matrix4f(1.f), Vector3f(0.f, 0.f, -5.f)));*/

	ECS::Entity cameraEntity = game.getECS().create();

	game.getECS().assign<TransformComponent>(cameraEntity, Matrix4f(1.f));
	game.getECS().assign<CameraComponent>(cameraEntity,
			&((GameRenderContext*)game.getRenderContext())->getCamera());

	addUpdateSystem(ShipPickBlockSystem(cameraEntity));

	ECS::Entity ship = game.getECS().create();
	game.getECS().assign<TransformComponent>(ship, Matrix4f(1.f));
	game.getECS().assign<Ship>(ship);
	game.getECS().assign<ShipBuildInfo>(ship, Matrix4f(1.f), BlockInfo::TYPE_BASIC_CUBE);

	Ship& shipComponent = game.getECS().get<Ship>(ship);

	Block block;
	block.type = BlockInfo::TYPE_BASIC_FIVE_SIXTH;

	for (uint32 i = 0; i < 3; ++i) {
		block.offset = Math::rotate(Math::translate(Matrix4f(1.f),
				Vector3f(i * BlockInfo::OFFSET_SCALE, 0.f,
				i * BlockInfo::OFFSET_SCALE)), Math::toRadians(90.f * i),
				Vector3f(0.f, 1.f, 0.f));
		shipComponent.blocks.push_back(block);
	}

	game.getECS().get<TransformComponent>(ship).transform =
			Math::rotate(Matrix4f(1.f), 0.3f, Vector3f(1.f, 1.f, 0.f));

	DEBUG_LOG_TEMP2("Loaded");
}

void GameScene::unload(Game& game) {
	DEBUG_LOG_TEMP2("Unloaded");
}

GameScene::~GameScene() {
	DEBUG_LOG_TEMP2("Removed");
}

static void renderMesh(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, RenderableMesh>().each([&](
			TransformComponent& transform, RenderableMesh& mesh) {
		((GameRenderContext*)game.getRenderContext())->renderMesh(*mesh.vertexArray,
				*mesh.material, transform.transform);
	});
}

static void renderSkybox(Game& game, float deltaTime) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

	Matrix4f mvp = Math::translate(grc->getCamera().viewProjection,
			Vector3f(grc->getCamera().view[3]));

	grc->getSkyboxCube().updateBuffer(1, &mvp, sizeof(Matrix4f));
	grc->renderSkybox(grc->getSpecularIBL(), grc->getLinearMipmapSampler());
}

static void toggleFullscreenSystem(Game& game, float deltaTime) {
	if (Application::getKeyPressed(Input::KEY_M)) {
		if (game.getWindow().isFullscreen()) {
			game.getWindow().setFullscreen(false);
			game.getWindow().resize(1200, 800);
			game.getWindow().moveToCenter();
		}
		else {
			game.getWindow().setFullscreen(true);
		}
	}
	else if (Application::getKeyPressed(Input::KEY_ESCAPE)
			&& game.getWindow().isFullscreen()) {
		game.getWindow().setFullscreen(false);
		game.getWindow().resize(1200, 800);
		game.getWindow().moveToCenter();
	}
}

