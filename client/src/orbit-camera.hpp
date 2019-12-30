#pragma once

struct CameraDistanceComponent {
	float distance;
	float minDistance;
	float maxDistance;
};

void orbitCameraSystem(float deltaTime);

