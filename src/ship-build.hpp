#pragma once

#include <engine/ecs/ecs.hpp>

#include "block.hpp"

class Game;

struct ShipBuildInfo {
	Matrix4f transform;
	enum BlockInfo::BlockType objectType;
};

class ShipBuildSystem {
	public:
		inline ShipBuildSystem(ECS::Entity cameraInfo)
				: cameraInfo(cameraInfo) {}

		void operator()(Game&, float);
	private:
		ECS::Entity cameraInfo;
};

class UpdateBuildToolTip {
	public:
		UpdateBuildToolTip(Game& game, ECS::Entity cameraInfo);

		void operator()(Game& game, float deltaTime);
	private:
		ECS::Entity toolTip;
		ECS::Entity cameraInfo;
};

void updateBuildToolTip(Game& game, float deltaTime);
void updateShipBuildInfo(Game& game, float deltaTime);

