#include "game-scene.hpp"

#include <engine/networking/server.hpp>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 40000

static float counter = 0.f;

void GameScene::load() {
	constexpr const uint8 PRIVATE_KEY[yojimbo::KeyBytes] = {0};

	NetworkServer::getInstance().start(SERVER_ADDRESS, SERVER_PORT,
			PRIVATE_KEY);
}

void GameScene::update(float deltaTime) {
	if (NetworkServer::getInstance().isRunning()) {
		NetworkServer::getInstance().sendMessages();
		NetworkServer::getInstance().receiveMessages();

		NetworkServer::getInstance().forEachClient([](auto& client) {
			StateUpdate su;

			if (client.getStateUpdate(su)) {
				printf("UPDATING STATE FOR FRAME %d:\n", su.sequence);
				printf("\tINPUT STATE: %d\n", su.inputState.data);

				for (uint32 i = 0; i < su.numBodies; ++i) {
					printf("\tnetworkID = %d\n", su.networkIDs[i]);
				}
			}
			else {
				puts("NO UPDATES FOR THIS FRAME");
			}
		});
	}
}

void GameScene::render() {}

void GameScene::unload() {
	NetworkServer::getInstance().stop();
}

