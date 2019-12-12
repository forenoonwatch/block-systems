#pragma once

#include "ship.hpp"

#include <engine/game/scene.hpp>

class GameScene : public Scene {
	public:
		GameScene();

		virtual void load(Game& game) override;
		virtual void unload(Game& game) override;

		virtual ~GameScene();
	private:
};

