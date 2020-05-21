#include "orbit-camera.hpp"

#include <engine/ecs/ecs.hpp>

#include <engine/game/util-components.hpp>
#include <engine/game/player-input.hpp>

#include <engine/core/application.hpp>
#include <engine/game/game-render-context.hpp>

#include <engine/math/math.hpp>

#define SCROLL_POWER 20.f

void orbitCameraSystem(float deltaTime) {
	static double lastScrollY = 0.0;

	const double scrollY = Application::getInstance().getScrollY();
	
	ECS::Registry::getInstance().view<TransformComponent, CameraComponent,
			CameraDistanceComponent, PlayerInputComponent>().each([&](auto& tf,
			auto& cc, auto& cdc, auto& pic) {
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
		cc.camera->view = Math::translate(Matrix4f(1.f), cc.position) * r;

		Application::getInstance().updateCameraBuffer();
	});

	lastScrollY = scrollY;
}

