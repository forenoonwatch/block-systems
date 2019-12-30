#pragma once

#include <engine/game/scene.hpp>

class TempScene : public Scene<TempScene> {
	public:
		void load();

		void update(float deltaTime);
		void render();

		void unload();
	private:
};

