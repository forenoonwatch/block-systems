#include "orbit-camera.hpp"

#include <engine/game/util-components.hpp>
#include <engine/game/game-render-context.hpp>

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

#include <engine/math/math.hpp>

#define SCROLL_POWER 20.f

void OrbitCameraSystem::operator()(Game& game, float deltaTime) {
	static double lastX = 0.0;
	static double lastY = 0.0;

	static double lastScrollY = 0.0;

	const double mouseX = Application::getMouseX();
	const double mouseY = Application::getMouseY();

	const double scrollY = Application::getScrollY();
	
	game.getECS().view<TransformComponent, CameraComponent,
			CameraDistanceComponent>().each([&](TransformComponent& tf,
			CameraComponent& cc, CameraDistanceComponent& cdc) {
		if (Application::isMouseDown(Input::MOUSE_BUTTON_RIGHT)) {
			cc.rotationX += (float)( (lastY - mouseY) * 0.01 );
			cc.rotationY += (float)( (lastX - mouseX) * 0.01 );

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

		((GameRenderContext*)game.getRenderContext())->updateCameraBuffer();
	});

	lastX = mouseX;
	lastY = mouseY;

	lastScrollY = scrollY;
}

