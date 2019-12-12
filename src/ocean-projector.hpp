#pragma once

#include <engine/game/camera.hpp>

class Game;

struct OceanProjector {
	Camera projectorCamera;
	Matrix4f mProjector;
	Vector4f corners[4];

	static const uint32 cameraEdges[24];
};

class UpdateOceanProjector : public ECS::System {
	public:
		virtual void operator()(Game& game, float deltaTime) override;
};

