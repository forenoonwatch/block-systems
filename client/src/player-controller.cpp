#include "player-controller.hpp"

#include <engine/ecs/registry.hpp>

#include <engine/components/transform-component.hpp>
#include <engine/components/player-input.hpp>

#include <engine/rendering/render-system.hpp>

void updatePlayerController(Registry& registry, float deltaTime) {
	auto& camera = RenderSystem::ref().getCamera();
	Vector3f rightVector = camera.getRightVector();
	Vector3f lookVector = camera.getLookVector();

	lookVector.y = rightVector.y = 0.f;
	rightVector = Math::normalize(rightVector);
	lookVector = -Math::normalize(lookVector);

	registry.view<PlayerController, PlayerInputComponent, TransformComponent>().each(
			[&](auto& pc, auto& pic, auto& tfc) {
		float x = 0.f, z = 0.f;

		if (pic.forward) {
			++z;
		}
		
		if (pic.back) {
			--z;
		}

		if (pic.left) {
			--x;
		}

		if (pic.right) {
			++x;
		}

		if (x == 0.f && z == 0.f) {
			pc.moveDirection = Vector3f();
		}
		else {
			pc.moveDirection = Math::normalize(rightVector * x + lookVector * z);
			const Vector3f newPos = tfc.transform.getPosition() + pc.moveDirection * (pc.moveSpeed * deltaTime);

			Transform tf(tfc.transform);
			tf.matrixLookAt(newPos);

			tfc.transform.setPosition(newPos);
			tfc.transform.setRotation(Math::slerp(tfc.transform.getRotation(), tf.getRotation(), 0.1f));
		}
	});
}
