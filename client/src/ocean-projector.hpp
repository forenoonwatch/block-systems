#pragma once

#include <engine/rendering/camera.hpp>

class Registry;

struct OceanProjector {
	Camera projectorCamera;
	Matrix4f mProjector;
	Vector4f corners[4];

	static const uint32 cameraEdges[24];
};

void updateOceanProjector(Registry& registry, Camera& camera, float deltaTime);

