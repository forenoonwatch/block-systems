#include <cstdio>
#include <signal.h>
#include <unistd.h>

#include <engine/networking/net-common.hpp>

#include <engine/networking/server.hpp>
#include <engine/networking/client.hpp>

#define SERVER_ADDRESS "127.0.0.1"

#define CLIENT_PORT 30000
#define SERVER_PORT 40000

static volatile int quit = 0;

static GameServer* serverPtr = nullptr;
static GameClient* clientPtr = nullptr;

void onInterrupted(int);

int main() {
	if (!InitializeYojimbo()) {
		puts("ERROR: failed to initialize yojimbo");
		return 1;
	}

	yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);

	signal(SIGINT, onInterrupted);

	constexpr const uint8_t PRIVATE_KEY[yojimbo::KeyBytes] = {0};

	puts("Enter S/s for server, C/c for client");

	char c = getchar();

	constexpr const double frameTime = 1.0 / 60.0;

	double lastTime = yojimbo_time();
	double currentTime, passedTime;

	double updateTimer = 1.0;

	if (c == 'S' || c == 's') {
		GameServer server(SERVER_ADDRESS, SERVER_PORT, PRIVATE_KEY);
		serverPtr = &server;

		for (; server.isRunning();) {
			currentTime = yojimbo_time();
			passedTime = currentTime - lastTime;
			lastTime = currentTime;

			updateTimer += passedTime;

			while (updateTimer >= frameTime) {
				server.receiveMessages();

				server.forEachClient([](auto& client) {
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

				server.sendMessages();

				updateTimer -= frameTime;
			}

			usleep(1000);
		}

		server.stop();
	}
	else if (c == 'C' || c == 'c') {
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
	}

	ShutdownYojimbo();

	return 0;
}

void onInterrupted(int) {
	quit = 1;

	if (serverPtr != nullptr) {
		serverPtr->stop();
	}

	if (clientPtr != nullptr) {
		clientPtr->stop();
	}
}

