#include "game-scene.hpp"

#include <engine/networking/server.hpp>
#include <engine/networking/network-object.hpp>

#include <engine/ecs/registry.hpp>
#include <engine/physics/physics-engine.hpp>

#include <engine/game/util-components.hpp>

#include <engine/core/memory.hpp>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 40000

static float counter = 0.f;

static void onMessageReceived(ClientConnection&, yojimbo::Message*);
static void onPlayerConnected(ClientConnection&);

void GameScene::load() {
	constexpr const uint8 PRIVATE_KEY[yojimbo::KeyBytes] = {0};

	NetworkServer::getInstance().setMessageCallback(::onMessageReceived);
	NetworkServer::getInstance().setConnectCallback(::onPlayerConnected);

	NetworkServer::getInstance().start(SERVER_ADDRESS, SERVER_PORT,
			PRIVATE_KEY);

	Physics::BodyHints bodyHints;
	bodyHints.type = Physics::BodyType::STATIC;
	bodyHints.collisionGroups = 3;
	bodyHints.transform = Transform();

	Physics::Body* body = Physics::PhysicsEngine::getInstance()
			.addBody(bodyHints);

	Physics::ColliderHints collHints;
	collHints.setRestitution(0.5f);
	collHints.setFriction(0.3f);
	collHints.setDensity(0.15f);
	collHints.setTransform(Transform());
	collHints.initPlane();
	body->addCollider(collHints);

	ECS::Entity ePlane = ECS::Registry::getInstance().create();
	ECS::Registry::getInstance().assign<TransformComponent>(ePlane);
	ECS::Registry::getInstance().assign<Physics::BodyHandle>(ePlane,
			Physics::BodyHandle(body));
}

void GameScene::update(float deltaTime) {
	if (NetworkServer::getInstance().isRunning()) {
		NetworkServer::getInstance().receiveMessages();

		NetworkServer::getInstance().forEachClient([](auto& client) {
			StateUpdate su;

			if (client.getStateUpdate(su)) {
				deserializeBodyStates(su);
			}

			serializeBodyStates(su);

			NetworkServer::getInstance()
					.broadcastMessageWithSystem<StateUpdateMessage>(
					GameMessageType::STATE_UPDATE_MESSAGE,
					GameChannelType::UNRELIABLE, [&](auto* msg) {
				Memory::memcpy(&msg->stateUpdate, &su, sizeof(StateUpdate));
			}, &client);
		});

		NetworkServer::getInstance().sendMessages();
	}
}

void GameScene::render() {}

void GameScene::unload() {
	NetworkServer::getInstance().stop();
}

static void onMessageReceived(ClientConnection& client,
		yojimbo::Message* msg) {
	switch (msg->GetType()) {
		case (int)GameMessageType::STATE_UPDATE_MESSAGE:
		{
			StateUpdateMessage* sum = static_cast<StateUpdateMessage*>(msg);
			sum->stateUpdate.sequence = msg->GetId();
			client.addStateUpdate(NetworkServer::getInstance().getTime(),
					msg->GetId(), sum->stateUpdate);
		}
			break;
		default:
			DEBUG_LOG_TEMP2("Got unknown message!");
	}
}

static void onPlayerConnected(ClientConnection& client) {
	auto plr = ECS::Registry::getInstance().create();
	const uint32 networkID = NetworkObject::counter++;

	Physics::BodyHints bodyHints;
	bodyHints.type = Physics::BodyType::DYNAMIC;
	bodyHints.transform = Transform(Vector3f(0.f, 10.f, 0.f));

	Physics::Body* body = Physics::PhysicsEngine::getInstance()
			.addBody(bodyHints);

	Physics::ColliderHints collHints;
	collHints.setRestitution(0.5f);
	collHints.setFriction(0.3f);
	collHints.setDensity(0.15f);
	collHints.initCapsule(Vector3f(0.f, -0.5f, 0.f), Vector3f(0.f, 0.5f, 0.f),
			1.f);
	body->addCollider(collHints);

	ECS::Registry::getInstance().assign<NetworkObject>(plr,
			client.getClientID(), networkID, 10, 0,
			NetworkRole::ROLE_AUTHORITATIVE, NetworkRole::ROLE_AUTONOMOUS);
	ECS::Registry::getInstance().assign<TransformComponent>(plr);
	ECS::Registry::getInstance().assign<Physics::BodyHandle>(plr,
			Physics::BodyHandle(body));

	// Send everyone the player's new character
	NetworkServer::getInstance()
			.broadcastMessageWithSystem<PlayerConnectMessage>(
			GameMessageType::PLAYER_CONNECT_MESSAGE,
			GameChannelType::RELIABLE, [&](auto* msg) {
		msg->clientID = client.getClientID();
		msg->networkID = networkID;
	});

	// send new player everyone else
	ECS::Registry::getInstance().view<NetworkObject, Physics::BodyHandle>()
			.each([&](auto& no, auto& handle) {
		if (no.networkOwner != client.getClientID()) {
			NetworkServer::getInstance()
					.sendDirectMessageWithSystem<PlayerConnectMessage>(
					GameMessageType::PLAYER_CONNECT_MESSAGE,
					GameChannelType::RELIABLE, [&](auto* msg) {
				msg->clientID = no.networkOwner;
				msg->networkID = no.networkID;
				// TODO: send initial state data?
			}, client);
		}
	});
}

