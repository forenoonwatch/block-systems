#include "orbit-camera.hpp"

#include <engine/application/application.hpp>
#include <engine/ecs/ecs.hpp>
#include <engine/rendering/render-system.hpp>

#include <engine/components/transform-component.hpp>
#include <engine/components/player-input.hpp>
#include <engine/components/camera-component.hpp>

#include <engine/math/math.hpp>

#define SCROLL_POWER 20.f

void orbitCameraSystem(Registry& registry, Application& app, RenderSystem& renderer, float deltaTime) {
	static double lastScrollY = 0.0;

	const double scrollY = app.getScrollY();

	auto& camera = renderer.getCamera();
	
	registry.view<TransformComponent, CameraComponent, CameraDistanceComponent,
			PlayerInputComponent>().each([&](auto& tf, auto& cc, auto& cdc, auto& pic) {
		if (pic.rightMouse) {
			cc.rotationX += pic.mouseDeltaY * -0.01f; 
			cc.rotationY += pic.mouseDeltaX * -0.01f;

			if (cc.rotationX < -1.27f) {
				cc.rotationX = -1.27f;
			}
			else if (cc.rotationX > 1.27f) {
				cc.rotationX = 1.27f;
			}
		}

		cdc.distance -= (float)(scrollY - lastScrollY) * SCROLL_POWER
				* deltaTime;
		cdc.distance = Math::clamp(cdc.distance, cdc.minDistance,
				cdc.maxDistance);

		Quaternion q = Math::rotate(Quaternion(1.f, 0.f, 0.f, 0.f),
				cc.rotationY, Vector3f(0.f, 1.f, 0.f));
		q = Math::rotate(q, cc.rotationX, Vector3f(1.f, 0.f, 0.f));

		Matrix4f r = Math::quatToMat4(q);

		cc.position = tf.transform.getPosition()
				+ Vector3f(r * Vector4f(0.f, 0.f, cdc.distance, 0.f));

		camera.view = Math::translate(Matrix4f(1.f), cc.position) * r;
	});

	lastScrollY = scrollY;
}
