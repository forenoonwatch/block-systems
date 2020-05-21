#include <csignal>

#include <engine/game/scene-manager.hpp>

#include "game-scene.hpp"

#define CLIENT_PORT 30000

void onInterrupted(int);

int main() {
	signal(SIGINT, onInterrupted);

	SceneManager::getInstance().load<GameScene>();

	/*else if (c == 'C' || c == 'c') {
		GameClient client(SERVER_ADDRESS, SERVER_PORT, PRIVATE_KEY);
		clientPtr = &client;

		for (int i = 0; i < 10; ++i) {
			client.addNetworkObject({0, (uint32_t)i, (uint32_t)i + 1, 0,
					NetworkRole::ROLE_AUTONOMOUS,
					NetworkRole::ROLE_AUTONOMOUS});
		}

		for (uint32_t inputCounter = 0;; ++inputCounter) {
			currentTime = yojimbo_time();
			passedTime = currentTime - lastTime;
			lastTime = currentTime;

			updateTimer += passedTime;
			
			while (updateTimer >= frameTime) {
				client.receiveMessages();

				if (!client.isConnected()) {
					break;
				}

				client.addInputState({inputCounter});

				client.sendMessages();

				updateTimer -= frameTime;
			}

			usleep(1000);
		}

		client.stop();
	}
	else {
		printf("Invalid input: %c\n", c);
	}*/

	return 0;
}

void onInterrupted(int) {
	SceneManager::getInstance().stop();
}

