#include "island.hpp"

#include "physics.hpp"

#include "body.hpp"
#include "contact-constraint.hpp"

#include <engine/math/math.hpp>

#include <cfloat>

#define NUM_ITERATIONS	10

#define SLEEP_LINEAR	0.01f
#define SLEEP_ANGULAR	((3.f / 180.f) * MATH_PI)
#define SLEEP_TIME		0.5f

static Quaternion integrateAngularVelocity(const Quaternion& rot,
		const Vector3f& angularVelocity, float deltaTime);

void Physics::Island::solve(float deltaTime) {
	// integrate velocities
	for (Body* body : bodies) {
		if (body->isDynamic()) {
			body->applyForce(Physics::GRAVITY * body->getGravityScale());

			const Matrix3f rot
					= Math::quatToMat3(body->transform.getRotation());
			body->invInertiaWorld = rot * body->invInertiaLocal
					* Math::transpose(rot);
			
			body->velocity += body->force * body->invMass * deltaTime;
			body->angularVelocity += body->invInertiaWorld * body->torque
					* deltaTime;
			
			// TODO: damping
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
		if (body->isStatic()) {
			continue;
		}

		body->worldCenter += body->velocity * deltaTime;
		body->transform.setRotation(integrateAngularVelocity(
				body->transform.getRotation(), body->angularVelocity,
				deltaTime));
	}

	// TODO: sleep
	float minSleepTime = FLT_MAX;

	for (Body* body : bodies) {
		if (body->isStatic()) {
			continue;
		}

		const float velSq = Math::dot(body->velocity, body->velocity);
		const float avSq = Math::dot(body->angularVelocity,
				body->angularVelocity);

		if (velSq > SLEEP_LINEAR || avSq > SLEEP_ANGULAR) {
			minSleepTime = 0.f;
			body->sleepTime = 0.f;
		}
		else {
			body->sleepTime += deltaTime;
			minSleepTime = Math::min(minSleepTime, body->sleepTime);
		}
	}

	if (minSleepTime > SLEEP_TIME) {
		for (Body* body : bodies) {
			body->setToSleep();
		}
	}
	
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

