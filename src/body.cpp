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
		, torque() {}

Physics::Body::Body(PhysicsEngine& physicsEngine, const BodyHints& hints)
		: physicsEngine(&physicsEngine)
		, transform()
		, localCenter()
		, worldCenter()
		, velocity(hints.velocity)
		, angularVelocity(hints.angularVelocity)
		, force(hints.force)
		, torque(hints.torque)
		, mass(0.f)
		, invMass(0.f)
		, invInertiaLocal(0.f)
		, invInertiaWorld(0.f)
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

void Physics::Body::addCollider(const ColliderHints& hints) {
	Collider* c = Collider::make(hints);
	c->body = this;

	colliders.push_back(c);

	calcMassData();

	physicsEngine->addCollider(*this, *c);
}

void Physics::Body::calcMassData() {
	if (isStatic() || isKinematic()) {
		mass = 0.f;
		invMass = 0.f;
		invInertiaLocal = Matrix3f(0.f);
		localCenter = Vector3f();

		return;
	}

	mass = 0.f;
	Matrix3f inertia(0.f);
	Vector3f centerOfMass(0.f, 0.f, 0.f);

	float colliderMass;
	Matrix3f colliderInertia;
	Vector3f colliderCOM;
	
	for (Collider* coll : colliders) {
		if (coll->getDensity() == 0.f) {
			continue;
		}

		coll->calcMassData(colliderMass, colliderInertia, colliderCOM);

		mass += colliderMass;
		inertia += colliderInertia;
		centerOfMass += colliderCOM * colliderMass;
	}

	if (mass > 0.f) {
		invMass = 1.f / mass;
		localCenter = centerOfMass * invMass;

		inertia -= (Matrix3f(Math::dot(localCenter, localCenter))
				- Math::outerProduct(localCenter, localCenter)) * mass;
		invInertiaLocal = Math::inverse(inertia);

		// TODO: axis locks
	}
	else {
		invMass = 1.f;
		invInertiaLocal = Matrix3f(0.f);
		localCenter = Vector3f(0.f);
	}
}

Physics::Body::~Body() {
	for (uint32 i = 0; i < colliders.size(); ++i) {
		delete colliders[i];
	}
}

