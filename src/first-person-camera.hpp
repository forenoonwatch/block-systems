#pragma once

#include <engine/ecs/ecs-system.hpp>

class Game;

class FirstPersonCameraSystem : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};

