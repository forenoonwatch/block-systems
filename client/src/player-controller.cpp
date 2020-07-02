#include "player-controller.hpp"

#include <engine/ecs/registry.hpp>

#include <engine/components/player-input.hpp>

#include <engine/rendering/render-system.hpp>

#include <engine/math/transform.hpp>

#include <engine/physics/character-controller.hpp>
#include <engine/physics/physics-util.hpp>

void updatePlayerController(Registry& registry, float deltaTime) {
	auto& camera = RenderSystem::ref().getCamera();
	Vector3f rightVector = camera.getRightVector();
	Vector3f lookVector = camera.getLookVector();

	lookVector.y = rightVector.y = 0.f;
	rightVector = Math::normalize(rightVector);
	lookVector = -Math::normalize(lookVector);

	//registry.view<PlayerController, PlayerInputComponent, Body>().each(
	//		[&](auto& pc, auto& pic, auto& body) {
	registry.view<PlayerController, PlayerInputComponent, CharacterController>().each(
			[&](auto& pc, auto& pic, auto& cc) {
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


			//Transform tf;
			//body.getCenterOfMassTransform(tf);

			//const Vector3f newPos = tf.getPosition() + pc.moveDirection * (pc.moveSpeed * deltaTime);

			//Transform newTf(tf);
			//newTf.matrixLookAt(newPos);

			//tf.setPosition(newPos);
			//tf.setRotation(Math::slerp(tf.getRotation(), newTf.getRotation(), 0.1f));

			//body.setCenterOfMassTransform(tf);

			//if (!body.isAwake()) {
			//	body.setToAwake();
			//}
		}

		cc.getController()->setWalkDirection(Physics::nativeToBtVec3(pc.moveDirection * (5.f * deltaTime)));

		if (pic.jump && cc.getController()->canJump()) {
			cc.getController()->jump();
			//body.applyImpulse(Vector3f(0, 5, 0));

			//if (!body.isAwake()) {
			//	body.setToAwake();
			//}
		}
	});
}
