#include "first-person-camera.hpp"

#include <engine/game/camera.hpp>

#include <engine/game/util-components.hpp>
#include <engine/game/player-input.hpp>
#include <engine/game/game-render-context.hpp>

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

#define CAMERA_SPEED 5.f

void FirstPersonCameraSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent,
			PlayerInputComponent>().each([&](auto& transform, auto& camera,
			auto& pic) {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (pic.forward) {
			z -= 1.f;
		}

		if (pic.back) {
			z += 1.f;
		}

		if (pic.left) {
			x -= 1.f;
		}

		if (pic.right) {
			x += 1.f;
		}

		// TODO: pic.up, pic.down
		if (game.getApplication().isKeyDown(Input::KEY_Q)) {
			y -= 1.f;
		}

		if (game.getApplication().isKeyDown(Input::KEY_E)) {
			y += 1.f;
		}

		if (pic.rightMouse) {
			camera.rotationX += pic.mouseDeltaY * -0.01f; 
			camera.rotationY += pic.mouseDeltaX * -0.01f; 

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
}

