#include <engine/game/scene-manager.hpp>

#include "temp-scene.hpp"

int main() {
	SceneManager::getInstance().load<TempScene>();

	return 0;
}

