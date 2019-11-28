#include "island.hpp"

#include "body.hpp"
#include "contact-constraint.hpp"

#define NUM_ITERATIONS 10

static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime);

void Physics::Island::solve(float deltaTime) {
	// integrate velocities
	for (Body* body : bodies) {
		if (body->isDynamic()) {
			const Matrix3f rot
					= Math::quatToMat3(body->transform.getRotation());
			body->invInertiaWorld = rot * body->invInertiaLocal
					* Math::transpose(rot);
			
			body->velocity += body->force * body->invMass * deltaTime;
			body->angularVelocity += body->invInertiaWorld * body->torque
					* deltaTime;
		}
	}

	// solve contacts
	for (ContactConstraint* constraint : contactList) {
		constraint->preSolve(deltaTime);
	}

	for (uint32 i = 0; i < NUM_ITERATIONS; ++i) {
		for (ContactConstraint* constraint : contactList) {
			constraint->solve();
		}
	}

	// integrate positions
	for (Body* body : bodies) {
		body->transform.setPosition(body->transform.getPosition()
				+ body->velocity * deltaTime);
		body->transform.setRotation(integrateAngularVelocity(
				body->transform.getRotation(), body->angularVelocity,
				deltaTime));
	}

	// TODO: sleep
	
	clearIslandFlags();
}

inline void Physics::Island::clearIslandFlags() {
	for (Body* body : bodies) {
		if (body->isStatic()) {
			body->flags &= ~Body::FLAG_ISLAND;
		}
	}
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

