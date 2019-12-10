#include "game-scene-2.hpp"

#include "camera.hpp"
//#include "first-person-camera.hpp"
#include "orbit-camera.hpp"

#include "renderable-mesh.hpp"

#include "util-components.hpp"
#include "util-systems.hpp"

#include "game-render-context.hpp"

#include "physics.hpp"
#include "body.hpp"

#include "sphere-collider.hpp"
#include "plane-collider.hpp"
#include "capsule-collider.hpp"
#include "convex-collider.hpp"

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

namespace {
	struct Player {
		float moveSpeed;
	};

	class PlayerControlSystem : public ECS::System {
		public:
			PlayerControlSystem(Physics::PhysicsEngine* pe,
						Physics::GravitySystem* gs)
					: pe(pe)
					, gs(gs) {}

			virtual void operator()(Game&, float) override;
		private:
			Physics::PhysicsEngine* pe;
			Physics::GravitySystem* gs;
	};

	void printCC(const Physics::ConvexCollider*);
};

GameScene2::GameScene2()
		: Scene()
		, physicsEngine(new Physics::PhysicsEngine())
		, gravitySystem(new Physics::GravitySystem())
		, sphereCollider(nullptr)
		, sphereCollider2(nullptr)
		, convexCollider(nullptr)
		, convexCollider2(nullptr)
		, planeCollider(nullptr)
		, capsuleCollider(nullptr) {

	//addUpdateSystem(new FirstPersonCameraSystem());
	addUpdateSystem(new OrbitCameraSystem());
	addUpdateSystem(new UpdateCameraSystem());
	addUpdateSystem(new ToggleFullscreenSystem());
	addUpdateSystem(new ::PlayerControlSystem(physicsEngine, gravitySystem));
	
	addUpdateSystem(gravitySystem);
	addUpdateSystem(physicsEngine);

	addRenderSystem(new RenderMesh());
	addRenderSystem(new GameRenderContext::Clear());
	addRenderSystem(new GameRenderContext::FlushStaticMeshes());
	addRenderSystem(new GameRenderContext::ApplyLighting());
	addRenderSystem(new RenderSkybox());
	addRenderSystem(new GameRenderContext::Flush());
}

void GameScene2::load(Game& game) {
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

	/*ECS::Entity cameraEntity = game.getECS().create();

	game.getECS().assign<TransformComponent>(cameraEntity, Transform());
	game.getECS().assign<CameraComponent>(cameraEntity,
			&((GameRenderContext*)game.getRenderContext())->getCamera());*/

	Physics::BodyHints bodyHints;
	bodyHints.mass = 1.f;
	bodyHints.type = Physics::BodyType::DYNAMIC;
	bodyHints.invInertiaLocal = Math::inverse(Matrix3f(0.4f));
	//bodyHints.invInertiaLocal = Math::inverse(Matrix3f(1.f / 6.f));
	// sphere I^-1 = diag(0.4f * M * R^2)^-1
	// cube I^1 = diag(mass / 6.f) * size

	// body 2
	Physics::Body* body2 = physicsEngine->addBody(bodyHints);
	
	//body2->invInertiaLocal[2] = Vector3f(0.f, 0.f, 0.f); // lock Z axis 

	//sphereCollider = new Physics::SphereCollider(1.f);
	//sphereCollider->setRestitution(0.f);
	//sphereCollider->setFriction(0.3f);
	//body2->setCollisionHull(sphereCollider);
	
	capsuleCollider = new Physics::CapsuleCollider(Vector3f(0.f, -0.5f, 0.f),
			Vector3f(0.f, 0.5f, 0.f), 1.f);
	capsuleCollider->setRestitution(0.f);
	capsuleCollider->setFriction(0.3f);
	body2->setCollisionHull(capsuleCollider);
	
	//convexCollider = new Physics::ConvexCollider(game.getAssetManager()
	//		.getModel("cube"));
	//convexCollider->restitution = 0.15f;
	//convexCollider->friction = 0.3f;
	//body2->setCollisionHull(convexCollider);

	Quaternion q = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(90.f), Vector3f(0.f, 0.f, 1.f)));

	ECS::Entity eSphere = game.getECS().create();
	game.getECS().assign<RenderableMesh>(eSphere,
			&game.getAssetManager().getVertexArray("capsule"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(eSphere,
			Transform(Vector3f(0.f, 20.f, 0.f), q, Vector3f(1.f)));
	game.getECS().assign<Physics::BodyHandle>(eSphere,
			Physics::BodyHandle(body2));

	// body 1 
	bodyHints.mass = 0.f;
	bodyHints.type = Physics::BodyType::STATIC;
	bodyHints.invInertiaLocal = Matrix3f(0.f);

	Physics::Body* body = physicsEngine->addBody(bodyHints);

	//planeCollider = new Physics::PlaneCollider();
	//planeCollider->setRestitution(0.1f);
	//planeCollider->setFriction(0.3f);
	//body->setCollisionHull(planeCollider);
	
	//sphereCollider2 = new Physics::SphereCollider(1.f);
	//sphereCollider2->setRestitution(0.1f);
	//sphereCollider2->setFriction(0.3f);
	//body->setCollisionHull(sphereCollider2);

	convexCollider2 = new Physics::ConvexCollider(game.getAssetManager()
			.getModel("platform"));
	convexCollider2->setRestitution(0.1f);
	convexCollider2->setFriction(0.3f);
	body->setCollisionHull(convexCollider2);

	//printCC(convexCollider2);

	//capsuleCollider2 = new Physics::CapsuleCollider(Vector3f(0.f, -0.5f, 0.f),
	//		Vector3f(0.f, 0.5f, 0.f), 1.f);
	//capsuleCollider2->setRestitution(0.1f);
	//capsuleCollider2->setFriction(0.3f);
	//body->setCollisionHull(capsuleCollider2);

	Quaternion rot = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(0.f), Vector3f(1.f, 0.f, 0.f)));

	ECS::Entity ePlane = game.getECS().create();
	game.getECS().assign<RenderableMesh>(ePlane,
			&game.getAssetManager().getVertexArray("platform"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(ePlane,
			Transform(Vector3f(0.f, 0.f, 0.f), rot,
			Vector3f(1.f, 1.f, 1.f)));
	game.getECS().assign<Physics::BodyHandle>(ePlane,
			Physics::BodyHandle(body));

	game.getECS().assign<CameraComponent>(eSphere,
			&((GameRenderContext*)game.getRenderContext())->getCamera());
	game.getECS().assign<CameraDistanceComponent>(eSphere, 2.f, 1.2f, 10.f);
	game.getECS().assign<Player>(eSphere, 0.1f);
}

void GameScene2::unload(Game& game) {
	if (convexCollider != nullptr) {
		delete convexCollider;
	}

	if (convexCollider2 != nullptr) {
		delete convexCollider2;
	}
	
	if (sphereCollider != nullptr) {
		delete sphereCollider;
	}
	
	if (sphereCollider2 != nullptr) {
		delete sphereCollider2;
	}

	if (planeCollider != nullptr) {
		delete planeCollider;
	}

	if (capsuleCollider != nullptr) {
		delete capsuleCollider;
	}

	if (capsuleCollider2 != nullptr) {
		delete capsuleCollider2;
	}
}

GameScene2::~GameScene2() {
}

void ::PlayerControlSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Player, CameraComponent,
			Physics::BodyHandle>().each([&](auto& tf,
			auto& plr, auto& cc, auto& handle) {
		Vector3f v(0.f, 0.f, 0.f);

		if (Application::isKeyDown(Input::KEY_W)) {
			v.z -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_S)) {
			v.z += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_A)) {
			v.x -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_D)) {
			v.x += 1.f;
		}

		v = Vector3f(cc.camera->view * Vector4f(v, 0.f));
		v.y = 0.f;
		v = Math::dot(v, v) < 0.000001f ? v : Math::normalize(v)
				* plr.moveSpeed;
		
		if (Application::isKeyDown(Input::KEY_SPACE)) {
			v.y += 0.5f;
		}

		if (Application::getKeyPressed(Input::KEY_N)) {
			(*gs)(game, deltaTime);
			(*pe)(game, deltaTime);
		}

		handle.body->getVelocity() += v;
		handle.body->setToAwake();
	});
}

namespace {
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

		DEBUG_LOG_TEMP("EDGES: %d", convexCollider->getEdges().size());

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

