#pragma once

#include <engine/game/scene.hpp>

class GameScene final : public Scene<GameScene> {
	public:
		void load();

		void update(float deltaTime);
		void render();

		void unload();
	private:
};

