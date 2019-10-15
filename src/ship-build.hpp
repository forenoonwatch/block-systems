#pragma once

#include <engine/core/array-list.hpp>

#include <engine/ecs/ecs.hpp>

#include "block.hpp"

class Game;

struct ShipBuildInfo {
	enum BlockInfo::BlockType objectType;
	Quaternion rotation;
	ArrayList<BlockInfo> blockInfo;
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

