#pragma once

#include <engine/ecs/ecs.hpp>

#include "block.hpp"

class Game;
class VertexArray;

struct Ship {
	ArrayList<Block> blocks;
};

struct ShipBuildInfo {
	Matrix4f transform;
	enum BlockInfo::BlockType objectType;
};

class ShipPickBlockSystem {
	public:
		inline ShipPickBlockSystem(ECS::Entity cameraInfo)
				: cameraInfo(cameraInfo) {}

		void operator()(Game&, float);
	private:
		ECS::Entity cameraInfo;
};

void updateShipBuildInfo(Game& game, float deltaTime);

void shipRenderSystem(Game& game, float deltaTime);

