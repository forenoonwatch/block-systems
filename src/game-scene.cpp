#include "game-scene.hpp"

#include "camera.hpp"
#include "renderable-mesh.hpp"
#include "util-components.hpp"

#include "game-render-context.hpp"

#include "ship-build.hpp"

#include "physics.hpp"

#include "ocean.hpp"
#include "ocean-projector.hpp"

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

#include <engine/ecs/ecs-system.hpp>

#define PHYSICS

namespace {
	class RenderMesh : public ECS::System {
		public:
			virtual void operator()(Game&, float) override;
	};

	class RenderSkybox : public ECS::System {
		public:
			virtual void operator()(Game&, float) override;
	};

	class RenderOcean : public ECS::System {
		public:
			virtual void operator()(Game&, float) override;
	};

	class ToggleFullscreenSystem : public ECS::System {
		public:
			virtual void operator()(Game&, float) override;
	};
};

struct ApplyImpulseSystem : public ECS::System {
	inline ApplyImpulseSystem(ECS::Entity cameraInfo)
			: cameraInfo(cameraInfo) {}

	void operator()(Game& game, float deltaTime) {
		if (Application::getKeyPressed(Input::KEY_X)) {
			const CameraComponent& cc =
					game.getECS().get<CameraComponent>(cameraInfo);

			Block* block;
			Vector3f mousePos;
			Vector3f hitNormal;

			game.getECS().view<TransformComponent, Physics::Body, Ship>()
					.each([&](TransformComponent& tf, Physics::Body& body,
					Ship& ship) {
				rayShipIntersection(tf.transform.toMatrix(), ship,
						cc.position, cc.rayDirection, block, &mousePos,
						&hitNormal);

				if (block != nullptr) {
					//Vector3f lp(tf.transform.inverse()
					//		* Vector4f(mousePos, 1.f));

					mousePos = Vector3f(tf.transform.toMatrix()
							* Vector4f(mousePos, 1.f));
					body.applyImpulse(cc.rayDirection, mousePos);
				}
			});
		}
		else if (Application::getKeyPressed(Input::KEY_Z)) {
			game.getECS().view<Physics::Body>().each([&](Physics::Body& body) {
				body.angularVelocity = Vector3f();
			});
		}
	}

	ECS::Entity cameraInfo;
};

GameScene::GameScene()
		: Scene() {
	addUpdateSystem(new FirstPersonCameraSystem());
	addUpdateSystem(new UpdateCameraSystem());
	addUpdateSystem(new UpdateOceanProjector());
	addUpdateSystem(new ToggleFullscreenSystem());
	addUpdateSystem(new UpdateShipBuildInfo());
#ifdef PHYSICS
	addUpdateSystem(new Physics::GravitySystem());
#endif

	addRenderSystem(new RenderMesh());

	addRenderSystem(new GameRenderContext::Clear());
	addRenderSystem(new GameRenderContext::FlushStaticMeshes());
	addRenderSystem(new ShipRenderSystem());
	addRenderSystem(new UpdateOceanBuffer());
	addRenderSystem(new RenderOcean());
	addRenderSystem(new GameRenderContext::ApplyLighting());
	addRenderSystem(new RenderSkybox());
	addRenderSystem(new GameRenderContext::Flush());
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

	game.getAssetManager().loadStaticMesh("plane", "plane",
			"./res/plane.obj", hints);
	
	game.getAssetManager().loadMaterial("bricks", "./res/bricks.dds",
			"./res/bricks-normal.dds", "./res/bricks-material.dds");
	game.getAssetManager().loadMaterial("wood-planks", "./res/wood-planks.dds",
			"./res/wood-planks-normal.dds", "./res/wood-planks-material.dds");
	game.getAssetManager().loadMaterial("plastic", "./res/plastic.dds",
			"./res/plastic-normal.dds", "./res/plastic-material.dds");

	game.getAssetManager().loadCubeMap("sargasso-diffuse",
			"./res/sargasso-diffuse.dds");
	game.getAssetManager().loadCubeMap("sargasso-specular",
			"./res/sargasso-specular.dds");
	game.getAssetManager().loadTexture("schlick-brdf",
			"./res/schlick-brdf.png");

	game.getAssetManager().loadTexture("foam", "./res/foam.jpg");

	game.getAssetManager().loadShader("ship-shader",
			"./res/shaders/block-deferred.glsl");
	game.getAssetManager().loadShader("ocean-deferred",
			"./res/shaders/ocean/ocean-deferred.glsl");

	grc->setDiffuseIBL(game.getAssetManager().getCubeMap("sargasso-diffuse"));
	grc->setSpecularIBL(game.getAssetManager()
			.getCubeMap("sargasso-specular"));
	grc->setBrdfLUT(game.getAssetManager().getTexture("schlick-brdf"));

	/*ECS::Entity e = game.getECS().create();
	game.getECS().assign<RenderableMesh>(e,
			&game.getAssetManager().getVertexArray("plane"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(e, Transform());*/

	ECS::Entity cameraEntity = game.getECS().create();

	game.getECS().assign<TransformComponent>(cameraEntity, Transform());
	game.getECS().assign<CameraComponent>(cameraEntity,
			&((GameRenderContext*)game.getRenderContext())->getCamera());

	game.getECS().assign<OceanProjector>(cameraEntity,
			((GameRenderContext*)game.getRenderContext())->getCamera(),
			Matrix4f(1.f));

	Memory::SharedPointer<OceanFFT> oceanFFT = Memory::make_shared<OceanFFT>(
			*game.getRenderContext(), 256, 1000, true, 6.f);

	oceanFFT->setOceanParams(2.f, Vector2f(1.f, 1.f), 10.f, 0.5f);

	game.getECS().assign<Ocean>(cameraEntity, oceanFFT);
	initOcean(*game.getRenderContext(),
			game.getECS().get<Ocean>(cameraEntity), 256);

	float f[] = {1.f, 0.01f, 1.f};
	game.getECS().get<Ocean>(cameraEntity).oceanDataBuffer->update(f,
			4 * sizeof(Vector4f), sizeof(f));

	addUpdateSystem(new ShipBuildSystem(cameraEntity));
	addUpdateSystem(new UpdateBuildToolTip(game, cameraEntity));
	addUpdateSystem(new ShipUpdateVAOSystem());

#ifdef PHYSICS
	addUpdateSystem(new ApplyImpulseSystem(cameraEntity));

	addUpdateSystem(new ShipUpdateMassSystem());
	addUpdateSystem(new ShipBuoyancySystem());
	addUpdateSystem(new Physics::IntegrateVelocities());
#endif

	ECS::Entity ship = game.getECS().create();
	game.getECS().assign<TransformComponent>(ship, Transform());
	game.getECS().assign<Ship>(ship);
	game.getECS().assign<ShipBuildInfo>(ship, (uint32)0,
			Quaternion(1.f, 0.f, 0.f, 0.f));

	game.getECS().assign<Physics::Body>(ship, Vector3f(), Vector3f(),
			Vector3f(), Vector3f(), Vector3f(0.f, -9.81f, 0.f), Vector3f(), 1.f, 1.f,
			Matrix3f(1.f), Matrix3f(1.f));

	Ship& shipComponent = game.getECS().get<Ship>(ship);

	BlockInfo::loadBlockInfo(game, "./res/block-info.csv");

	BlockInfo::initVertexArrays(*game.getRenderContext(),
			shipComponent.blockArrays);
	BlockInfo::initVertexArrays(*game.getRenderContext(),
			game.getECS().get<ShipBuildInfo>(ship).blockArrays);

	/*Block block;
	
	constexpr const int32 n = 1;

	for (int32 x = 0; x < n; ++x) {
		for (int32 y = 0; y < n; ++y) {
			for (int32 z = 0; z < n; ++z) {
				//block.type = (enum BlockInfo::BlockType)((x + y + z)
				//		% BlockInfo::NUM_TYPES);
				block.type = 0;
				block.position = Vector3i(x, y, z);
				block.rotation = Quaternion(1.f, 0.f, 0.f, 0.f);

				shipComponent.addBlock(block.type, block.position,
						block.rotation);
			}
		}
	}*/

	shipComponent.load("./res/test-ship.csv");

	/*for (int32 i = 0; i < 5; ++i) {
		shipComponent.addBlock(BlockInfo::TYPE_BASIC_CUBE,
				Vector3i(0, i, 0), Quaternion(1.f, 0.f, 0.f, 0.f));
	}

	for (int32 i = 1; i <= 2; ++i) {
		shipComponent.addBlock(BlockInfo::TYPE_BASIC_CUBE,
				Vector3i(i, 4, 0), Quaternion(1.f, 0.f, 0.f, 0.f));
	}*/

	game.getECS().get<TransformComponent>(ship)
			.transform.setPosition(Vector3f(0.f, 2.4f, 0.f));

	DEBUG_LOG_TEMP2("Loaded");
}

void GameScene::unload(Game& game) {
	DEBUG_LOG_TEMP2("Unloaded");
}

GameScene::~GameScene() {
	DEBUG_LOG_TEMP2("Removed");
}

void RenderMesh::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, RenderableMesh>().each([&](
			TransformComponent& transform, RenderableMesh& mesh) {
		if (mesh.render) {
			((GameRenderContext*)game.getRenderContext())->renderMesh(
					*mesh.vertexArray, *mesh.material,
					transform.transform.toMatrix());
		}
	});
}

void RenderSkybox::operator()(Game& game, float deltaTime) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

	Matrix4f mvp = Math::translate(grc->getCamera().viewProjection,
			Vector3f(grc->getCamera().view[3]));

	grc->getSkyboxCube().updateBuffer(1, &mvp, sizeof(Matrix4f));
	grc->renderSkybox(grc->getSpecularIBL(), grc->getLinearMipmapSampler());
}

void RenderOcean::operator()(Game& game, float deltaTime) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();
	Shader& oceanShader = game.getAssetManager().getShader("ocean-deferred");
	Texture& foam = game.getAssetManager().getTexture("foam");

	game.getECS().view<Ocean>().each([&](Ocean& ocean) {
		oceanShader.setSampler("displacementMap",
				ocean.oceanFFT->getDisplacement(), grc->getLinearSampler(), 0);
		oceanShader.setSampler("foldingMap", ocean.oceanFFT->getFoldingMap(),
				grc->getLinearSampler(), 1);
		oceanShader.setSampler("foam", foam, grc->getLinearSampler(), 2);

		grc->draw(grc->getTarget(), oceanShader, *ocean.gridArray,
				GL_TRIANGLES);
	});
}

void ToggleFullscreenSystem::operator()(Game& game, float deltaTime) {
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
	else if (Application::getKeyPressed(Input::KEY_B)) {
		game.getECS().view<TransformComponent, Ship>().each([](
				TransformComponent& tfc, Ship& ship) {
			tfc.transform.setPosition(Vector3f(0.f, 10.f, 0.f));
			tfc.transform.setRotation(Quaternion(1.f, 0.f, 0.f, 0.f));
		});
	}
}

