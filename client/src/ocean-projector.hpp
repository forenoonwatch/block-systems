#pragma once

#include <engine/game/camera.hpp>

class Game;

struct OceanProjector {
	Camera projectorCamera;
	Matrix4f mProjector;
	Vector4f corners[4];

	static const uint32 cameraEdges[24];
};

void updateOceanProjector(float deltaTime);

