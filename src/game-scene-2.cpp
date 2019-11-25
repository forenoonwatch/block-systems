#include "game-scene-2.hpp"

#include "camera.hpp"
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

GameScene2::GameScene2()
		: Scene()
		, physicsEngine(new Physics::PhysicsEngine())
		, sphereCollider(nullptr)
		, sphereCollider2(nullptr)
		, convexCollider(nullptr) {

	addUpdateSystem(new FirstPersonCameraSystem());
	addUpdateSystem(new UpdateCameraSystem());
	addUpdateSystem(new ToggleFullscreenSystem());
	
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

	ECS::Entity cameraEntity = game.getECS().create();

	game.getECS().assign<TransformComponent>(cameraEntity, Transform());
	game.getECS().assign<CameraComponent>(cameraEntity,
			&((GameRenderContext*)game.getRenderContext())->getCamera());

	// body 2
	Physics::Body* body2 = physicsEngine->addBody();
	body2->mass = body2->invMass = 1.f;
	body2->invInertiaLocal = body2->invInertiaWorld
			= Math::inverse(Matrix3f(0.4f));
	//body2->flags = Physics::Body::FLAG_STATIC;
	// sphere I^-1 = diag(0.4f * M * R^2)^-1

	sphereCollider = new Physics::SphereCollider();
	body2->collisionHull = sphereCollider;
	sphereCollider->body = body2;
	sphereCollider->radius = 1.f;
	sphereCollider->restitution = 0.f;
	sphereCollider->friction = 1.f;

	ECS::Entity eSphere = game.getECS().create();
	game.getECS().assign<RenderableMesh>(eSphere,
			&game.getAssetManager().getVertexArray("sphere"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(eSphere,
			Transform(Vector3f(0.00001f, 20.f, 0.f)));
	game.getECS().assign<Physics::BodyHandle>(eSphere,
			Physics::BodyHandle(body2));

	// body 1 
	Physics::Body* body = physicsEngine->addBody();
	body->mass = body->invMass = 0.f;
	body->invInertiaLocal = body->invInertiaWorld = Matrix3f(0.f);
	body->flags = Physics::Body::FLAG_STATIC;

	convexCollider = new Physics::ConvexCollider();
	body->collisionHull = convexCollider;
	convexCollider->body = body;
	convexCollider->restitution = 0.f;
	convexCollider->friction = 1.f;
	
	//sphereCollider2 = new Physics::SphereCollider();
	//body->collisionHull = sphereCollider2;
	//sphereCollider2->body = body;
	//sphereCollider2->radius = 1.f;
	//sphereCollider2->restitution = 0.f;

	Quaternion rot = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(5.f), Vector3f(0.f, 0.f, 1.f)));

	ECS::Entity ePlane = game.getECS().create();
	game.getECS().assign<RenderableMesh>(ePlane,
			&game.getAssetManager().getVertexArray("plane"),
			&game.getAssetManager().getMaterial("bricks"),
			true);
	game.getECS().assign<TransformComponent>(ePlane, Transform(Vector3f(),
			rot, Vector3f(1.f, 1.f, 1.f)));
	game.getECS().assign<Physics::BodyHandle>(ePlane,
			Physics::BodyHandle(body));
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

