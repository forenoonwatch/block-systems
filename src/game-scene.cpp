#include "game-scene.hpp"

#include "camera.hpp"
#include "renderable-mesh.hpp"
#include "util-components.hpp"

#include "game-render-context.hpp"

#include "ship-build.hpp"

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

static void renderMesh(Game&, float);
static void renderSkybox(Game&, float);
static void toggleFullscreenSystem(Game&, float);

static void initBlockTypes(Game&);

GameScene::GameScene()
		: Scene() {
	addUpdateSystem(::firstPersonCameraSystem);
	addUpdateSystem(::updateCameraSystem);
	addUpdateSystem(::toggleFullscreenSystem);
	addUpdateSystem(::updateShipBuildInfo);

	addRenderSystem(::renderMesh);

	addRenderSystem(GameRenderContext::clear);
	addRenderSystem(GameRenderContext::flushStaticMeshes);
	addRenderSystem(::shipRenderSystem);
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
	hints.elementSizes.push_back(16);
	hints.instancedElementStartIndex = 4;

	game.getAssetManager().loadStaticMesh("cube", "cube", "./res/cube.obj", hints);
	game.getAssetManager().loadStaticMesh("tetrahedron", "tetrahedron",
			"./res/tetrahedron.obj", hints);
	game.getAssetManager().loadStaticMesh("pyramid", "pyramid",
			"./res/pyramid.obj", hints);
	game.getAssetManager().loadStaticMesh("wedge", "wedge", "./res/wedge.obj", hints);
	game.getAssetManager().loadStaticMesh("five-sixths-block", "five-sixths-block",
			"./res/five-sixths-block.obj", hints);
	game.getAssetManager().loadStaticMesh("wedge-2x-1", "wedge-2x-1",
			"./res/wedge-2x-1.obj", hints);
	game.getAssetManager().loadStaticMesh("wedge-2x-2", "wedge-2x-2",
			"./res/wedge-2x-2.obj", hints);
	
	game.getAssetManager().loadMaterial("bricks", "./res/bricks.dds",
			"./res/bricks-normal.dds", "./res/bricks-material.dds");
	game.getAssetManager().loadMaterial("wood-planks", "./res/wood-planks.dds",
			"./res/wood-planks-normal.dds", "./res/wood-planks-material.dds");
	game.getAssetManager().loadCubeMap("sargasso-diffuse", "./res/sargasso-diffuse.dds");
	game.getAssetManager().loadCubeMap("sargasso-specular", "./res/sargasso-specular.dds");
	game.getAssetManager().loadTexture("schlick-brdf", "./res/schlick-brdf.png");

	game.getAssetManager().loadShader("ship-shader", "./res/shaders/static-mesh-deferred.glsl");

	grc->setDiffuseIBL(game.getAssetManager().getCubeMap("sargasso-diffuse"));
	grc->setSpecularIBL(game.getAssetManager().getCubeMap("sargasso-specular"));
	grc->setBrdfLUT(game.getAssetManager().getTexture("schlick-brdf"));

	initBlockTypes(game);

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

	addUpdateSystem(ShipBuildSystem(cameraEntity));
	addUpdateSystem(UpdateBuildToolTip(game, cameraEntity));

	ECS::Entity ship = game.getECS().create();
	game.getECS().assign<TransformComponent>(ship, Matrix4f(1.f));
	game.getECS().assign<Ship>(ship);
	game.getECS().assign<ShipBuildInfo>(ship, Matrix4f(1.f),
			BlockInfo::TYPE_BASIC_CUBE);

	Ship& shipComponent = game.getECS().get<Ship>(ship);

	Block block;
	//block.offset = Matrix4f(1.f);
	//block.type = BlockInfo::TYPE_BASIC_CUBE;
	//shipComponent.blocks.push_back(block);
	
	constexpr const uint32 n = 60;

	for (uint32 x = 0; x < n; ++x) {
		for (uint32 y = 0; y < n; ++y) {
			for (uint32 z = 0; z < n; ++z) {
				//block.type = (enum BlockInfo::BlockType)((x + y + z)
				//		% BlockInfo::NUM_TYPES);
				block.type = BlockInfo::TYPE_BASIC_CUBE;
				block.position = Vector3i(x, y, z);
				block.rotation = Vector2i(x % 4, y % 4);
				block.renderIndex = (uint32)-1;
				//block.offset = Math::translate(Matrix4f(1.f),
				//		Vector3f(x, y, z) * BlockInfo::OFFSET_SCALE);

				shipComponent.blocks[block.position] = block;
				shipComponent.hitTree.addObject(block.position);
				//shipComponent.offsets[block.type].push_back(block.offset);
			}
		}
	}

	//shipComponent.blocks[Vector3i(0, 3, 3)].type = BlockInfo::TYPE_BASIC_WEDGE;

	const Vector3i directions[] = {Vector3i(-1, 0, 0), Vector3i(1, 0, 0),
			Vector3i(0, -1, 0), Vector3i(0, 1, 0), Vector3i(0, 0, -1),
			Vector3i(0, 0, 1)};

	for (auto& pair : shipComponent.blocks) {
		/*bool occluded = true;

		for (uint32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(directions); ++i) {
			auto it = shipComponent.blocks.find(pair.first + directions[i]);

			if (it == shipComponent.blocks.end()) {
				occluded = false;
				break;
			}

			if ((BlockInfo::getInfo(it->second.type).flags
					& BlockInfo::FLAG_OCCLUDES) == 0) {
				occluded = false;
				break;
			}
		}*/

		//if (!occluded) {
			Matrix4f rot = Math::rotate(Matrix4f(1.f),
					Math::toRadians(90.f * pair.second.rotation.x),
					Vector3f(1.f, 0.f, 0.f));
			rot = Math::rotate(rot, Math::toRadians(90.f
					* pair.second.rotation.y), Vector3f(0.f, 1.f, 0.f));

			pair.second.renderIndex = shipComponent
					.offsets[pair.second.type].size();
			shipComponent.offsets[pair.second.type].push_back(
					Math::translate(Matrix4f(1.f), Vector3f(pair.first))
					* rot);
			shipComponent.offsetIndices[pair.second.type]
					.push_back(&pair.second);
		//}
	}

	for (auto& pair : shipComponent.offsets) {
		const_cast<VertexArray*>(BlockInfo::getInfo(pair.first).vertexArray)
				->updateBuffer(4, &pair.second[0], pair.second.size()
				* sizeof(Matrix4f));
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
		if (mesh.render) {
			((GameRenderContext*)game.getRenderContext())->renderMesh(*mesh.vertexArray,
					*mesh.material, transform.transform);
		}
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

inline static void initBlockTypes(Game& game) {
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_CUBE,
			BlockInfo::FLAG_OCCLUDES,
			game.getAssetManager().getModel("cube"),
			game.getAssetManager().getVertexArray("cube"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_TETRA,
			0,
			game.getAssetManager().getModel("tetrahedron"),
			game.getAssetManager().getVertexArray("tetrahedron"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_PYRAMID,
			0,
			game.getAssetManager().getModel("pyramid"),
			game.getAssetManager().getVertexArray("pyramid"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE,
			0,
			game.getAssetManager().getModel("wedge"),
			game.getAssetManager().getVertexArray("wedge"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_FIVE_SIXTH,
			0,
			game.getAssetManager().getModel("five-sixths-block"),
			game.getAssetManager().getVertexArray("five-sixths-block"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE_2X1,
			0,
			game.getAssetManager().getModel("wedge-2x-1"),
			game.getAssetManager().getVertexArray("wedge-2x-1"),
			game.getAssetManager().getMaterial("wood-planks"));
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE_2X1,
			0,
			game.getAssetManager().getModel("wedge-2x-2"),
			game.getAssetManager().getVertexArray("wedge-2x-2"),
			game.getAssetManager().getMaterial("wood-planks"));
}

