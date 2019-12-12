#include "first-person-camera.hpp"

#include <engine/game/camera.hpp>

#include <engine/game/util-components.hpp>
#include <engine/game/game-render-context.hpp>

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

#define CAMERA_SPEED 5.f

void FirstPersonCameraSystem::operator()(Game& game, float deltaTime) {
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

		Matrix4f tf = Math::rotate(Matrix4f(1.f), camera.rotationY,
				Vector3f(0.f, 1.f, 0.f));

		camera.position += Vector3f(tf[0]) * (x * CAMERA_SPEED * deltaTime)
				+ Vector3f(tf[2]) * (z * CAMERA_SPEED * deltaTime);
		camera.position.y += y * CAMERA_SPEED * deltaTime;

		tf *= Math::rotate(Matrix4f(1.f), camera.rotationX,
				Vector3f(1.f, 0.f, 0.f));
		tf = Math::translate(Matrix4f(1.f), camera.position) * tf;

		transform.transform.setPosition(Vector3f(tf[3]));
		transform.transform.setRotation(Math::mat4ToQuat(tf));
		
		camera.camera->view = transform.transform.toMatrix();

		((GameRenderContext*)game.getRenderContext())->updateCameraBuffer();
	});

	lastX = mouseX;
	lastY = mouseY;
}

