#pragma once

#include <engine/game/scene.hpp>

class GameScene2 final : public Scene<GameScene2> {
	public:
		void load();

		void update(float deltaTime);
		void render();

		void unload();
};

