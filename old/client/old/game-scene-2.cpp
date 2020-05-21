#include "game-scene-2.hpp"

//#include "first-person-camera.hpp"
#include "orbit-camera.hpp"

#include <engine/game/camera.hpp>
#include <engine/game/renderable-mesh.hpp>
#include <engine/game/util-components.hpp>
#include <engine/game/util-systems.hpp>

#include <engine/game/player-input.hpp>
#include <engine/game/physics-motion-controller.hpp>

#include <engine/physics/physics-engine.hpp>
#include <engine/physics/dynamics/body.hpp>

#include <engine/physics/collision/sphere-collider.hpp>
#include <engine/physics/collision/plane-collider.hpp>
#include <engine/physics/collision/capsule-collider.hpp>
#include <engine/physics/collision/convex-collider.hpp>

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

//#include <engine/game/game-render-context.hpp>

namespace {
	struct Player {
		float moveSpeed;
	};

	void renderPhysicsMeshes();

	void printCC(const Physics::ConvexCollider*);
};

void GameScene2::load() {
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
	game.getAssetManager().loadStaticMesh("sphere", "sphere",
			"./res/sphere.obj", hints);
	game.getAssetManager().loadStaticMesh("cube", "cube",
			"./res/cube.obj", hints);
	game.getAssetManager().loadStaticMesh("capsule", "capsule",
			"./res/capsule.obj", hints);

	game.getAssetManager().loadStaticMesh("platform", "platform",
			"./res/platform.obj", hints);
	
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

	game.getAssetManager().loadShader("ocean-deferred",
			"./res/shaders/ocean/ocean-deferred.glsl");

	grc->setDiffuseIBL(game.getAssetManager().getCubeMap("sargasso-diffuse"));
	grc->setSpecularIBL(game.getAssetManager()
			.getCubeMap("sargasso-specular"));
	grc->setBrdfLUT(game.getAssetManager().getTexture("schlick-brdf"));

	/*ECS::Entity cameraEntity = Services::ecs->create();

	Services::ecs->assign<TransformComponent>(cameraEntity, Transform());
	Services::ecs->assign<CameraComponent>(cameraEntity,
			&((GameRenderContext*)game.getRenderContext())->getCamera());*/

	Physics::BodyHints bodyHints;
	bodyHints.type = Physics::BodyType::DYNAMIC;
	bodyHints.gravityScale = 2.f;
	bodyHints.angularDamping = 0.2f;
	bodyHints.collisionGroups = 2;

	Quaternion q = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(90.f), Vector3f(0.f, 0.f, 1.f)));
	bodyHints.transform = Transform(Vector3f(0.f, 15.f, 0.f), q,
			Vector3f(1.f));

	Physics::ColliderHints collHints;
	collHints.setRestitution(0.5f);
	collHints.setFriction(0.3f);

	// body 2
	Physics::Body* body2 = Services::physicsEngine->addBody(bodyHints);

	Quaternion q1Local = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(90.f), Vector3f(0.f, 0.f, 1.f)));

	collHints.setDensity(0.15f);
	///collHints.initCapsule(Vector3f(0.f, -0.5f, 0.f),
	//		Vector3f(0.f, 0.5f, 0.f), 1.f);
	//collHints.initSphere(1.f);
	collHints.initConvexHull(game.getAssetManager().getModel("cube"));
	collHints.setTransform(Transform(Vector3f(), q1Local, Vector3f(1.f)));
	body2->addCollider(collHints);

	collHints.setTransform(Transform(Vector3f(5.f, 0.f, 0.f)));
	body2->addCollider(collHints);

	ECS::Entity eSphere = Services::ecs->create();
	Services::ecs->assign<RenderableMesh>(eSphere,
			&game.getAssetManager().getVertexArray("capsule"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	Services::ecs->assign<TransformComponent>(eSphere,
			Transform(Vector3f(0.f, 10.f, 0.f), q, Vector3f(1.f)));
	Services::ecs->assign<Physics::BodyHandle>(eSphere,
			Physics::BodyHandle(body2));

	// body 1 
	bodyHints.type = Physics::BodyType::STATIC;
	bodyHints.collisionGroups = 3;
	bodyHints.transform = Transform();

	Physics::Body* body = Services::physicsEngine->addBody(bodyHints);

	collHints.setTransform(Transform());
	///collHints.initConvexHull(game.getAssetManager().getModel("platform"));
	collHints.initPlane();
	body->addCollider(collHints);

	Quaternion rot = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(0.f), Vector3f(1.f, 0.f, 0.f)));

	ECS::Entity ePlane = Services::ecs->create();
	Services::ecs->assign<RenderableMesh>(ePlane,
			&game.getAssetManager().getVertexArray("platform"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	Services::ecs->assign<TransformComponent>(ePlane,
			Transform(Vector3f(0.f, 0.f, 0.f), rot,
			Vector3f(1.f, 1.f, 1.f)));
	Services::ecs->assign<Physics::BodyHandle>(ePlane,
			Physics::BodyHandle(body));

	Services::ecs->assign<CameraComponent>(eSphere,
			&((GameRenderContext*)game.getRenderContext())->getCamera());
	Services::ecs->assign<CameraDistanceComponent>(eSphere, 2.f, 1.2f, 10.f);
	//Services::ecs->assign<Player>(eSphere, 0.1f);
	Services::ecs->assign<PlayerInputComponent>(eSphere);
}

void GameScene2::unload(Game& game) {
	
}

namespace {
	void renderPhysicsMeshes() {
		Material& material = game.getAssetManager().getMaterial("bricks");

		Services::ecs->view<TransformComponent, Physics::BodyHandle>().each([&](
				auto& tf, auto& handle) {
			for (const auto* coll : handle.body->getColliders()) {
				switch (coll->getType()) {
					case Physics::ColliderType::TYPE_SPHERE:
						grc->renderMesh(game.getAssetManager()
								.getVertexArray("sphere"), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_PLANE:
						grc->renderMesh(game.getAssetManager()
								.getVertexArray("plane"), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_CAPSULE:
						grc->renderMesh(game.getAssetManager()
								.getVertexArray("capsule"), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_CONVEX_HULL:
						grc->renderMesh(game.getAssetManager()
								.getVertexArray("cube"), material,
								coll->getWorldTransform().toMatrix());
						break;
				}
			}
		});
	}

	void printCC(const Physics::ConvexCollider* convexCollider) {
		DEBUG_LOG_TEMP2("VERTICES");

		for (const auto& v : convexCollider->getVertices()) {
			DEBUG_LOG_TEMP("%.2f, %.2f, %.2f", v.x, v.y, v.z);
		}

		DEBUG_LOG_TEMP2("FACES");

		for (uint32 i = 0; i < convexCollider->getFaces().size(); ++i) {
			const auto& f = convexCollider->getFaces()[i];

			DEBUG_LOG_TEMP("F %d:", i);

			Vector3f v = f.centroid;
			DEBUG_LOG_TEMP("C: %.2f, %.2f, %.2f", v.x, v.y, v.z);
			v = f.normal;
			DEBUG_LOG_TEMP("N: %.2f, %.2f, %.2f", v.x, v.y, v.z);

			for (const auto& ep : f.edgePlanes) {
				v = ep.normal;
				DEBUG_LOG_TEMP("\tFN: %.2f, %.2f, %.2f", v.x, v.y, v.z);
				DEBUG_LOG_TEMP("\t\tEID: %d", ep.edgeID);
			}

			for (const auto& v : f.vertices) {
				DEBUG_LOG_TEMP("\tFV: %.2f, %.2f, %.2f", v.v.x, v.v.y, v.v.z);
				DEBUG_LOG_TEMP("\t\tEID: %d", v.edgeID);
			}
		}

		DEBUG_LOG_TEMP("EDGES: %ld", convexCollider->getEdges().size());

		for (uint32 i = 0; i < convexCollider->getEdges().size(); ++i) {
			const auto& e = convexCollider->getEdges()[i];

			Vector3f v = (e.v1 + e.v0) * 0.5f;
			DEBUG_LOG_TEMP("EC %d: %.2f, %.2f, %.2f", i, v.x, v.y, v.z);
		}

		DEBUG_LOG_TEMP2("FACE AXES");

		for (const auto& a : convexCollider->getFaceAxes()) {
			Vector3f v = a.axis;
			DEBUG_LOG_TEMP("FA: %.2f, %.2f, %.2f", v.x, v.y, v.z);

			for (const auto& i : a.indices) {
				DEBUG_LOG_TEMP("\t%d", i);
			}
		}

		DEBUG_LOG_TEMP2("EDGE AXES");

		for (const auto& a : convexCollider->getEdgeAxes()) {
			Vector3f v = a.axis;
			DEBUG_LOG_TEMP("EA: %.2f, %.2f, %.2f", v.x, v.y, v.z);

			for (const auto& i : a.indices) {
				DEBUG_LOG_TEMP("\t%d", i);
			}
		}
	}
};

