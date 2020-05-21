#include "first-person-camera.hpp"

#include <engine/rendering/camera.hpp>

#include <engine/components/transform-component.hpp>
#include <engine/components/player-input.hpp>
#include <engine/components/camera-component.hpp>

#include <engine/application/application.hpp>
#include <engine/ecs/ecs.hpp>
#include <engine/rendering/render-system.hpp>

#define CAMERA_SPEED 5.f

void firstPersonCameraSystem(Registry& registry, Application& app, RenderSystem& renderer, float deltaTime) {
	auto& camera = renderer.getCamera();

	registry.view<TransformComponent, CameraComponent, PlayerInputComponent>().each([&](
			auto& transform, auto& cc, auto& pic) {
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
		if (app.isKeyDown(Input::KEY_Q)) {
			y -= 1.f;
		}

		if (app.isKeyDown(Input::KEY_E)) {
			y += 1.f;
		}

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

		Matrix4f tf = Math::rotate(Matrix4f(1.f), cc.rotationY,
				Vector3f(0.f, 1.f, 0.f));

		cc.position += Vector3f(tf[0]) * (x * CAMERA_SPEED * deltaTime)
				+ Vector3f(tf[2]) * (z * CAMERA_SPEED * deltaTime);
		cc.position.y += y * CAMERA_SPEED * deltaTime;

		tf *= Math::rotate(Matrix4f(1.f), cc.rotationX,
				Vector3f(1.f, 0.f, 0.f));
		tf = Math::translate(Matrix4f(1.f), cc.position) * tf;

		transform.transform.setPosition(Vector3f(tf[3]));
		transform.transform.setRotation(Math::mat4ToQuat(tf));
		
		camera.view = transform.transform.toMatrix();
	});
}
