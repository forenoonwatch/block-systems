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

#include "convex-collider.hpp"
#include "sphere-collider.hpp"

#include <engine/core/application.hpp>
#include <engine/math/math.hpp>

namespace {
	struct Player {
		float moveSpeed;
	};

	class PlayerControlSystem : public ECS::System {
		public:
			virtual void operator()(Game&, float) override;
	};
};

GameScene2::GameScene2()
		: Scene()
		, physicsEngine(new Physics::PhysicsEngine())
		, sphereCollider(nullptr)
		, sphereCollider2(nullptr)
		, convexCollider(nullptr) {

	//addUpdateSystem(new FirstPersonCameraSystem());
	addUpdateSystem(new OrbitCameraSystem());
	addUpdateSystem(new UpdateCameraSystem());
	addUpdateSystem(new ToggleFullscreenSystem());
	addUpdateSystem(new ::PlayerControlSystem());
	
	addUpdateSystem(new Physics::GravitySystem());
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

	// body 2
	Physics::Body* body2 = physicsEngine->addBody(bodyHints);
	//body2->invInertiaLocal = body2->invInertiaWorld
	//		= Math::inverse(Matrix3f(0.4f));
	//body2->invInertiaLocal[2] = Vector3f(0.f, 0.f, 0.f); // lock Z axis 
	// sphere I^-1 = diag(0.4f * M * R^2)^-1

	sphereCollider = new Physics::SphereCollider();
	sphereCollider->radius = 1.f;
	sphereCollider->restitution = 0.f;
	sphereCollider->friction = 0.3f;
	body2->setCollisionHull(sphereCollider);

	ECS::Entity eSphere = game.getECS().create();
	game.getECS().assign<RenderableMesh>(eSphere,
			&game.getAssetManager().getVertexArray("sphere"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(eSphere,
			Transform(Vector3f(0.00001f, 1.f, 0.f)));
	game.getECS().assign<Physics::BodyHandle>(eSphere,
			Physics::BodyHandle(body2));

	// body 1 
	bodyHints.mass = 0.f;
	bodyHints.type = Physics::BodyType::STATIC;

	Physics::Body* body = physicsEngine->addBody(bodyHints);

	convexCollider = new Physics::ConvexCollider();
	convexCollider->restitution = 0.f;
	convexCollider->friction = 0.3f;
	body->setCollisionHull(convexCollider);
	
	//sphereCollider2 = new Physics::SphereCollider();
	//body->collisionHull = sphereCollider2;
	//sphereCollider2->body = body;
	//sphereCollider2->radius = 1.f;
	//sphereCollider2->restitution = 0.f;

	Quaternion rot = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(0.f), Vector3f(0.f, 0.f, 1.f)));

	ECS::Entity ePlane = game.getECS().create();
	game.getECS().assign<RenderableMesh>(ePlane,
			&game.getAssetManager().getVertexArray("plane"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(ePlane, Transform(Vector3f(),
			rot, Vector3f(1.f, 1.f, 1.f)));
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
	
	if (sphereCollider != nullptr) {
		delete sphereCollider;
	}
	
	if (sphereCollider2 != nullptr) {
		delete sphereCollider2;
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
			v.y += 1.f;
		}

		handle.body->getVelocity() += v;
	});
}

