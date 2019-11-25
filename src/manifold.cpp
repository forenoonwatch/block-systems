#include "manifold.hpp"

#include "body.hpp"
#include "collision.hpp"

#include <engine/math/math.hpp>

using namespace Physics;

#define BAUMGARTE			0.2f
#define PENETRATION_SLOP	0.05f

static void computeBasis(const Vector3f& normal, Vector3f& tangent0,
		Vector3f& tangent1);

Physics::Manifold::Manifold(Body& a, Body& b)
		: bodyA(&a)
		, bodyB(&b)
		, numContacts(0)
		, friction(mixFriction())
		, restitution(mixRestitution()) {}

bool Physics::Manifold::testCollision() {
	COLLISION_DISPATCH[bodyA->collisionHull->type][bodyB->collisionHull->type]
			(*this, *(bodyA->collisionHull), *(bodyB->collisionHull));
	return numContacts > 0;
}

void Physics::Manifold::preSolve(float deltaTime) {
	computeBasis(normal, tangents[0], tangents[1]);

	for (uint32 i = 0; i < numContacts; ++i) {
		ContactState& cs = contacts[i];

		Vector3f dv = bodyB->velocity
				+ Math::cross(bodyB->angularVelocity, cs.rB) - bodyA->velocity
				- Math::cross(bodyA->angularVelocity, cs.rA);

		// calculate normal and tangent mass
		cs.normalMass = bodyA->invMass + bodyB->invMass;
		cs.tangentMass[0] = cs.tangentMass[1] = cs.normalMass;

		Vector3f rAcN = Math::cross(cs.rA, normal);
		Vector3f rBcN = Math::cross(cs.rB, normal);

		cs.normalMass += Math::dot(rAcN, bodyA->invInertiaWorld * rAcN)
				+ Math::dot(rBcN, bodyB->invInertiaWorld * rBcN);
		cs.normalMass = cs.normalMass == 0.f ? 0.f : 1.f / cs.normalMass;

		for (uint32 j = 0; j < 2; ++j) {
			Vector3f rAcT = Math::cross(cs.rA, tangents[j]);
			Vector3f rBcT = Math::cross(cs.rB, tangents[j]);

			cs.tangentMass[j] += Math::dot(rAcT, bodyA->invInertiaWorld * rAcT)
				+ Math::dot(rBcT, bodyB->invInertiaWorld * rBcT);
			cs.tangentMass[j] = cs.tangentMass[j] == 0.f ? 0.f
					: 1.f / cs.tangentMass[j];
		}

		// calculate bias
		cs.bias = -BAUMGARTE / deltaTime
				* Math::min(0.f, cs.penetration + PENETRATION_SLOP);
		
		// TODO: properly implement accumulated impulse
		cs.normalImpulse = cs.tangentImpulse[0] = cs.tangentImpulse[1] = 0.f;
		
		// warm start
		Vector3f p = normal * cs.normalImpulse;
		p += tangents[0] * cs.tangentImpulse[0];
		p += tangents[1] * cs.tangentImpulse[1];

		bodyA->velocity -= p * bodyA->invMass;
		bodyA->angularVelocity -= bodyA->invInertiaWorld
				* Math::cross(cs.rA, p);
		
		bodyB->velocity += p * bodyB->invMass;
		bodyB->angularVelocity += bodyB->invInertiaWorld
				* Math::cross(cs.rB, p);

		// add restitution bias
		float rv = Math::dot(dv, normal);

		if (rv < -1.f) {
			cs.bias += -restitution * rv;
		}
	}
}

void Physics::Manifold::solve() {
	if (bodyA->invMass == 0.f && bodyB->invMass == 0.f) {
		correctInfiniteMass();
		return;
	}

	for (uint32 i = 0; i < numContacts; ++i) {
		ContactState& cs = contacts[i];

		// TODO: swap order of normal and tangent impulse
		Vector3f dv = bodyB->velocity
				+ Math::cross(bodyB->angularVelocity, cs.rB) - bodyA->velocity
				- Math::cross(bodyA->angularVelocity, cs.rA);

		float rv = Math::dot(dv, normal);

		float lambda = cs.normalMass * (-rv + cs.bias);

		float oldNI = cs.normalImpulse;
		cs.normalImpulse = Math::max(oldNI + lambda, 0.f);
		lambda = cs.normalImpulse - oldNI;

		Vector3f impulse = normal * lambda;

		bodyA->velocity -= impulse * bodyA->invMass;
		bodyA->angularVelocity -= bodyA->invInertiaWorld
				* Math::cross(cs.rA, impulse);

		bodyB->velocity += impulse * bodyB->invMass;
		bodyB->angularVelocity += bodyB->invInertiaWorld
				* Math::cross(cs.rB, impulse);

		dv = bodyB->velocity + Math::cross(bodyB->angularVelocity, cs.rB)
				- bodyA->velocity - Math::cross(bodyA->angularVelocity, cs.rA);

		for (uint32 j = 0; j < 2; ++j) {
			lambda = -Math::dot(dv, tangents[j]) * cs.tangentMass[j];

			float oldTI = cs.tangentImpulse[j];
			cs.tangentImpulse[j] = Math::clamp(oldTI + lambda,
					-cs.normalImpulse * friction, cs.normalImpulse * friction);
			lambda = cs.tangentImpulse[j] - oldTI;

			impulse = tangents[j] * lambda;

			bodyA->velocity -= impulse * bodyA->invMass;
			bodyA->angularVelocity -= bodyA->invInertiaWorld
					* Math::cross(cs.rA, impulse);

			bodyB->velocity += impulse * bodyB->invMass;
			bodyB->angularVelocity += bodyB->invInertiaWorld
					* Math::cross(cs.rB, impulse);
		}
	}
}

void Physics::Manifold::addContact(const Vector3f& point, float penetration) {
	ContactState& cs = contacts[numContacts];
	cs.rA = point - bodyA->transform.getPosition();
	cs.rB = point - bodyB->transform.getPosition();
	cs.penetration = penetration;

	++numContacts;
}

inline void Physics::Manifold::correctInfiniteMass() {
	bodyA->velocity = bodyB->velocity = Vector3f();
}

inline float Physics::Manifold::mixFriction() {
	return Math::sqrt(bodyA->collisionHull->friction
			* bodyB->collisionHull->friction);
}

inline float Physics::Manifold::mixRestitution() {
	return Math::max(bodyA->collisionHull->restitution,
			bodyB->collisionHull->restitution);
}

inline static void computeBasis(const Vector3f& normal, Vector3f& tangent0,
		Vector3f& tangent1) {
	if (normal.x >= 0.57735027f || normal.x <= -0.57735027f) {
		tangent0.x = normal.y;
		tangent0.y = -normal.x;
		tangent0.z = 0.f;
	}
	else {
		tangent0.x = 0.f;
		tangent0.y = normal.z;
		tangent0.z = -normal.y;
	}

	tangent0 = Math::normalize(tangent0);
	tangent1 = Math::cross(normal, tangent0);
}

