#pragma once

class Registry;
class Application;
class RenderSystem;

struct CameraDistanceComponent {
	float distance;
	float minDistance;
	float maxDistance;
};

void orbitCameraSystem(Registry& registry, Application& app, RenderSystem& renderer, float deltaTime);
