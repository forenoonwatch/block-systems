#include "camera.hpp"

#include "util-components.hpp"
#include "game-render-context.hpp"

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

#define CAMERA_SPEED 5.f

void updateCameraSystem(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& cc) {
		Camera& camera = *cc.camera;

		camera.view = transform.transform;
		camera.iView = Math::inverse(camera.view);
		
		camera.viewProjection = camera.projection * camera.iView;
		camera.iViewProjection = Math::inverse(camera.viewProjection);
	});
}

void firstPersonCameraSystem(Game& game, float deltaTime) {
	static double lastX = 0.0;
	static double lastY = 0.0;

	const double mouseX = Application::getMouseX();
	const double mouseY = Application::getMouseY();

	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& camera) {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (Application::isKeyDown(Input::KEY_W)) {
			z -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_S)) {
			z += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_A)) {
			x -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_D)) {
			x += 1.f;
		}

		if (Application::isKeyDown(Input::KEY_Q)) {
			y -= 1.f;
		}

		if (Application::isKeyDown(Input::KEY_E)) {
			y += 1.f;
		}

		if (Application::isMouseDown(Input::MOUSE_BUTTON_RIGHT)) {
			camera.rotationX += (float)( (lastY - mouseY) * 0.01 );
			camera.rotationY += (float)( (lastX - mouseX) * 0.01 );

			if (camera.rotationX < -1.27f) {
				camera.rotationX = -1.27f;
			}
			else if (camera.rotationX > 1.27f) {
				camera.rotationX = 1.27f;
			}
		}

		transform.transform = Math::rotate(Matrix4f(1.f), camera.rotationY, Vector3f(0.f, 1.f, 0.f));

		camera.position += Vector3f(transform.transform[0]) * (x * CAMERA_SPEED * deltaTime)
				+ Vector3f(transform.transform[2]) * (z * CAMERA_SPEED * deltaTime);
		camera.position.y += y * CAMERA_SPEED * deltaTime;

		transform.transform *= Math::rotate(Matrix4f(1.f), camera.rotationX, Vector3f(1.f, 0.f, 0.f));
		transform.transform = Math::translate(Matrix4f(1.f), camera.position) * transform.transform;

		((GameRenderContext*)game.getRenderContext())->updateCameraBuffer();
	});

	lastX = mouseX;
	lastY = mouseY;
}

