#pragma once

#include <engine/core/memory.hpp>
#include <engine/core/array-list.hpp>

#include <engine/ecs/ecs.hpp>
#include <engine/ecs/ecs-system.hpp>

#include "block.hpp"

class Game;

struct ShipBuildInfo {
	uint32 objectType;
	Quaternion rotation;
	ArrayList<Memory::SharedPointer<VertexArray>> blockArrays;
};

class ShipBuildSystem : public ECS::System {
	public:
		inline ShipBuildSystem(ECS::Entity cameraInfo)
				: cameraInfo(cameraInfo) {}

		virtual void operator()(Game& game, float deltaTime) override;
	private:
		ECS::Entity cameraInfo;
};

class UpdateBuildToolTip : public ECS::System {
	public:
		UpdateBuildToolTip(Game& game, ECS::Entity cameraInfo);

		virtual void operator()(Game& game, float deltaTime) override;
	private:
		ECS::Entity toolTip;
		ECS::Entity cameraInfo;
};

class UpdateShipBuildInfo : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};

