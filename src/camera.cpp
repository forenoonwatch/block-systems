#include "camera.hpp"

#include "util-components.hpp"
#include "game-render-context.hpp"

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

#define CAMERA_SPEED 5.f

void UpdateCameraSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& cc) {
		Camera& camera = *cc.camera;

		camera.view = transform.transform.toMatrix();
		camera.iView = Math::inverse(camera.view);

		const Matrix4f iProjection = Math::inverse(camera.projection);
		
		camera.viewProjection = camera.projection * camera.iView;
		//camera.iViewProjection = Math::inverse(camera.viewProjection);
		camera.iViewProjection = camera.view * iProjection;

		// mouse ray
		const float ndcX = (2.f * Application::getMouseX())
				/ (float)game.getWindow().getWidth() - 1.f;
		const float ndcY = (2.f * Application::getMouseY())
				/ (float)game.getWindow().getHeight() - 1.f;

		Vector4f rawRay = iProjection
				* Vector4f(ndcX, -ndcY, -1.f, 1.f);
		rawRay = camera.view * Vector4f(rawRay.x, rawRay.y, -1.f, 0.f);
	
		cc.rayDirection = Math::normalize(Vector3f(rawRay));
	});
}

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

		((GameRenderContext*)game.getRenderContext())->updateCameraBuffer();
	});

	lastX = mouseX;
	lastY = mouseY;
}

