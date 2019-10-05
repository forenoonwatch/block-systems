#pragma once

#include <engine/ecs/ecs.hpp>

#include "block.hpp"

class Game;
class VertexArray;

struct Ship {
	ArrayList<Block> blocks;
};

class ShipPickBlockSystem {
	public:
		inline ShipPickBlockSystem(ECS::Entity cameraInfo)
				: cameraInfo(cameraInfo) {}

		void operator()(Game&, float);
	private:
		ECS::Entity cameraInfo;
};

void shipRenderSystem(Game& game, float deltaTime);

