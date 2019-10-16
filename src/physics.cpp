#include "physics.hpp"

#include "util-components.hpp"

#include <engine/game/game.hpp>

static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime);

void Physics::integrateVelocities(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Physics::Body>().each([&](
			TransformComponent& tf, Physics::Body& body) {
		body.worldCenter = Vector3f(tf.transform.toMatrix()
				* Vector4f(body.localCenter, 1.f));

		const Matrix3f rot = Math::quatToMat3(tf.transform.getRotation());
		body.invInertiaWorld = rot * body.invInertiaLocal
				* Math::transpose(rot);
			
		body.velocity += body.force * body.invMass * deltaTime;
		body.angularVelocity += body.invInertiaWorld * body.torque * deltaTime;

		//body.velocity = Vector3f(); // TODO: remove

		tf.transform.setPosition(tf.transform.getPosition()
				+ body.velocity * deltaTime);
		tf.transform.setRotation(integrateAngularVelocity(
				tf.transform.getRotation(), body.angularVelocity, deltaTime));
	});
}

inline static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime) {
	// wxyz
	Quaternion q(0.f, angularVelocity.x * deltaTime,
			angularVelocity.y * deltaTime, angularVelocity.z * deltaTime);

	q *= rot;
	q = rot + q * 0.5f;

	return Math::normalize(q);
}

