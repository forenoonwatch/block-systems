#include "body.hpp"

#include "collider.hpp"
#include "physics.hpp"

Physics::BodyHints::BodyHints()
		: type(BodyType::DYNAMIC)
		, active(true)
		, awake(true)
		, allowSleep(true)
		, velocity()
		, angularVelocity()
		, force()
		, torque()
		, mass(1.f)
		, invInertiaLocal(0.f) {}

Physics::Body::Body(PhysicsEngine& physicsEngine, const BodyHints& hints)
		: physicsEngine(&physicsEngine)
		, transform()
		, localCenter()
		, worldCenter()
		, velocity(hints.velocity)
		, angularVelocity(hints.angularVelocity)
		, force(hints.force)
		, torque(hints.torque)
		, mass(hints.mass)
		, invMass(hints.mass == 0.f ? 0.f : 1.f / hints.mass)
		, invInertiaLocal(hints.invInertiaLocal)
		, invInertiaWorld(hints.invInertiaLocal)
		, sleepTime(0.f)
		, flags(0) {
	switch (hints.type) {
		case BodyType::DYNAMIC:
			flags |= FLAG_DYNAMIC;
			break;
		case BodyType::STATIC:
			flags |= FLAG_STATIC;
			velocity = Vector3f();
			angularVelocity = Vector3f();
			force = Vector3f();
			torque = Vector3f();

			break;
		case BodyType::KINEMATIC:
			flags |= FLAG_KINEMATIC;
			break;
	}

	if (hints.active) {
		flags |= FLAG_ACTIVE;
	}

	if (hints.awake) {
		flags |= FLAG_AWAKE;
	}

	if (hints.allowSleep) {
		flags |= FLAG_ALLOW_SLEEP;
	}
}

void Physics::Body::setCollider(Collider* collider) {
	this->collider = collider;
	collider->body = this;

	physicsEngine->addCollider(*this, *collider);
}

