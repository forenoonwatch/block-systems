#pragma once

#include <engine/ecs/ecs-system.hpp>

class Game;

struct CameraDistanceComponent {
	float distance;
	float minDistance;
	float maxDistance;
};

class OrbitCameraSystem : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};

