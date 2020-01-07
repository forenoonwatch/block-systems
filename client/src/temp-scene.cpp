#include <engine/core/hashed-string.hpp>

#include <engine/ecs/ecs.hpp>
#include <engine/core/application.hpp>
#include <engine/physics/physics-engine.hpp>
#include <engine/ecs/registry.hpp>

#include <engine/networking/client.hpp>
#include <engine/networking/network-object.hpp>
#include <engine/networking/state-update-message.hpp>

#include <engine/game/texture-loader.hpp>
#include <engine/game/cube-map-loader.hpp>
#include <engine/game/material-loader.hpp>

#include <engine/game/model-loader.hpp>
#include <engine/game/vertex-array-loader.hpp>

#include <engine/game/resource-cache.hpp>

#include <engine/game/util-components.hpp>
#include <engine/game/util-systems.hpp>
#include <engine/game/player-input.hpp>
#include <engine/game/physics-motion-controller.hpp>

#include "orbit-camera.hpp"

#include "temp-scene.hpp"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 40000

namespace {
	void renderPhysicsMeshes();
	void updateNetworkClient(float deltaTime);
	void onMessageReceived(yojimbo::Message*);
	void onPlayerConnected(PlayerConnectMessage* msg);
};

void TempScene::load() {
	constexpr const uint8 PRIVATE_KEY[yojimbo::KeyBytes] = {0};

	NetworkClient::getInstance().setMessageCallback(::onMessageReceived);

	NetworkClient::getInstance().connect(SERVER_ADDRESS, SERVER_PORT,
			PRIVATE_KEY);

	Application::getInstance().resizeWindow(1200, 800);
	Application::getInstance().moveToCenter();

	struct IndexedModel::AllocationHints hints;
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(2);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(16);
	hints.instancedElementStartIndex = 4;

	ResourceCache<VertexArray>::getInstance()
			.load<VertexArrayLoader>("sphere"_hs, "./res/sphere.obj", hints);
	ResourceCache<VertexArray>::getInstance()
			.load<VertexArrayLoader>("plane"_hs, "./res/plane.obj", hints);
	ResourceCache<VertexArray>::getInstance()
			.load<VertexArrayLoader>("cube"_hs, "./res/cube.obj", hints);
	ResourceCache<VertexArray>::getInstance()
			.load<VertexArrayLoader>("capsule"_hs, "./res/capsule.obj",
			hints);
	
	ResourceCache<VertexArray>::getInstance()
			.load<VertexArrayLoader>("platform"_hs, "./res/platform.obj",
			hints);

	ResourceCache<IndexedModel>::getInstance()
			.load<ModelLoader>("cube"_hs, "./res/cube.obj", hints);

	auto bricksDiffuse = ResourceCache<Texture>::getInstance()
			.load<TextureLoader>("bricks-diffuse"_hs, "./res/bricks.dds");
	auto bricksNormal = ResourceCache<Texture>::getInstance()
			.load<TextureLoader>("bricks-normal"_hs,
			"./res/bricks-normal.dds");
	auto bricksMaterial = ResourceCache<Texture>::getInstance()
			.load<TextureLoader>("bricks-material"_hs,
			"./res/bricks-material.dds");

	ResourceCache<Material>::getInstance().load<MaterialLoader>("bricks"_hs,
			bricksDiffuse, bricksNormal, bricksMaterial);

	String cubeMap = "./res/sargasso-diffuse.dds";
	ResourceCache<CubeMap>::getInstance()
			.load<CubeMapLoader>("sargasso-diffuse"_hs, &cubeMap, 1);
	cubeMap = "./res/sargasso-specular.dds";
	ResourceCache<CubeMap>::getInstance()
			.load<CubeMapLoader>("sargasso-specular"_hs, &cubeMap, 1);

	ResourceCache<Texture>::getInstance()
			.load<TextureLoader>("schlick-brdf"_hs, "./res/schlick-brdf.png");

	Application::getInstance().setDiffuseIBL(
			ResourceCache<CubeMap>::getInstance()
			.handle("sargasso-diffuse"_hs));
	Application::getInstance().setSpecularIBL(
			ResourceCache<CubeMap>::getInstance()
			.handle("sargasso-specular"_hs));
	Application::getInstance().setBrdfLUT(
			ResourceCache<Texture>::getInstance()
			.handle("schlick-brdf"_hs));

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

	// Body 2
	Physics::Body* body2 = Physics::PhysicsEngine::getInstance()
			.addBody(bodyHints);

	Quaternion q1Local = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(90.f), Vector3f(0.f, 0.f, 1.f)));
	collHints.setDensity(0.15f);
	collHints.initConvexHull(ResourceCache<IndexedModel>::getInstance()
			.handle("cube"_hs));
	collHints.setTransform(Transform(Vector3f(), q1Local, Vector3f(1.f)));
	body2->addCollider(collHints);

	collHints.setTransform(Transform(Vector3f(5.f, 0.f, 0.f)));
	body2->addCollider(collHints);

	// Body 1
	bodyHints.type = Physics::BodyType::STATIC;
	bodyHints.collisionGroups = 3;
	bodyHints.transform = Transform();

	Physics::Body* body = Physics::PhysicsEngine::getInstance()
			.addBody(bodyHints);

	collHints.setTransform(Transform());
	collHints.initPlane();
	body->addCollider(collHints);
	
	Quaternion rot = Math::mat4ToQuat(Math::rotate(Matrix4f(1.f),
			Math::toRadians(0.f), Vector3f(1.f, 0.f, 0.f)));

	ECS::Entity ePlane = ECS::Registry::getInstance().create();
	ECS::Registry::getInstance().assign<TransformComponent>(ePlane,
			Transform(Vector3f(0.f, 0.f, 0.f), rot,
			Vector3f(1.f, 1.f, 1.f)));
	ECS::Registry::getInstance().assign<Physics::BodyHandle>(ePlane,
			Physics::BodyHandle(body));

}

void TempScene::update(float deltaTime) {
	Application::getInstance().pollEvents();

	playerInputSystem(deltaTime);

	updateNetworkClient(deltaTime);

	orbitCameraSystem(deltaTime);
	updateCameraSystem(deltaTime);
	toggleFullscreenSystem(deltaTime);
	physicsMotionController(deltaTime);

	Physics::PhysicsEngine::getInstance().step(deltaTime);
}

void TempScene::render() {
	renderPhysicsMeshes();

	Application::getInstance().clear();
	Application::getInstance().flushStaticMeshes();
	Application::getInstance().applyLighting();
	
	renderSkybox();

	Application::getInstance().flush();

	Application::getInstance().render();
}

void TempScene::unload() {
	NetworkClient::getInstance().disconnect();
}

namespace {
	void renderPhysicsMeshes() {
		auto material = ResourceCache<Material>::getInstance()
				.handle("bricks"_hs);

		ECS::Registry::getInstance().view<TransformComponent,
				Physics::BodyHandle>().each([&](auto& tf, auto& handle) {
			for (const auto* coll : handle.body->getColliders()) {
				switch (coll->getType()) {
					case Physics::ColliderType::TYPE_SPHERE:
						Application::getInstance().renderMesh(
								ResourceCache<VertexArray>::getInstance()
								.handle("sphere"_hs), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_PLANE:
						Application::getInstance().renderMesh(
								ResourceCache<VertexArray>::getInstance()
								.handle("plane"_hs), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_CAPSULE:
						Application::getInstance().renderMesh(
								ResourceCache<VertexArray>::getInstance()
								.handle("capsule"_hs), material,
								coll->getWorldTransform().toMatrix());
						break;
					case Physics::ColliderType::TYPE_CONVEX_HULL:
						Application::getInstance().renderMesh(
								ResourceCache<VertexArray>::getInstance()
								.handle("cube"_hs), material,
								coll->getWorldTransform().toMatrix());
						break;
				}
			}
		});
	}

	void updateNetworkClient(float deltaTime) {
		NetworkClient::getInstance().receiveMessages();

		if (NetworkClient::getInstance().isConnected()) {
			if (NetworkClient::getInstance().canSendMessage(
					GameChannelType::UNRELIABLE)) {
				NetworkClient::getInstance()
						.sendMessageWithSystem<StateUpdateMessage>(
						GameMessageType::STATE_UPDATE_MESSAGE,
						GameChannelType::UNRELIABLE, [&](auto* msg) {
					writeInputStates(&msg->stateUpdate.inputState);
					serializeBodyStates(msg->stateUpdate);
				});
			}

			NetworkClient::getInstance().sendMessages();
		}
	}

	void onMessageReceived(yojimbo::Message* msg) {
		switch (msg->GetType()) {
			case (int)GameMessageType::STATE_UPDATE_MESSAGE:
				puts("Got state update message");
				break;
			case (int)GameMessageType::PLAYER_CONNECT_MESSAGE:
				onPlayerConnected(static_cast<PlayerConnectMessage*>(msg));
				break;
			default:
				puts("Whatever");
		}
	}

	void onPlayerConnected(PlayerConnectMessage* msg) {
		auto plr = ECS::Registry::getInstance().create();

		Physics::BodyHints bodyHints;
		bodyHints.type = Physics::BodyType::DYNAMIC;
		bodyHints.transform = Transform(Vector3f(0.f, 10.f, 0.f));

		Physics::Body* body = Physics::PhysicsEngine::getInstance()
				.addBody(bodyHints);

		Physics::ColliderHints collHints;
		collHints.setRestitution(0.5f);
		collHints.setFriction(0.3f);
		collHints.setDensity(0.15f);
		collHints.initCapsule(Vector3f(0.f, -0.5f, 0.f),
				Vector3f(0.f, 0.5f, 0.f), 1.f);
		body->addCollider(collHints);

		ECS::Registry::getInstance().assign<TransformComponent>(plr);
		ECS::Registry::getInstance().assign<Physics::BodyHandle>(plr,
				Physics::BodyHandle(body));

		if (msg->clientID == NetworkClient::getInstance().getClientID()) {
			ECS::Registry::getInstance().assign<NetworkObject>(plr,
					msg->clientID, msg->networkID, 10, 0,
					NetworkRole::ROLE_AUTONOMOUS,
					NetworkRole::ROLE_AUTHORITATIVE);

			ECS::Registry::getInstance().assign<CameraComponent>(plr,
					&Application::getInstance().getCamera());
			ECS::Registry::getInstance().assign<CameraDistanceComponent>(plr,
					2.f, 1.2f, 10.f);
			ECS::Registry::getInstance().assign<PlayerInputComponent>(plr);
		}
		else {
			ECS::Registry::getInstance().assign<NetworkObject>(plr,
					msg->clientID, msg->networkID, 0, 0,
					NetworkRole::ROLE_AUTONOMOUS,
					NetworkRole::ROLE_AUTHORITATIVE);
		}
	}
};

