#include "contact-manager.hpp"

#include "physics.hpp"
#include "body.hpp"
#include "collision-hull.hpp"

static void computeBasis(const Vector3f& a, Vector3f* b, Vector3f* c);

Physics::ContactManager::ContactManager(PhysicsEngine& physicsEngine)
		: physicsEngine(&physicsEngine) {}

void Physics::ContactManager::testCollisions() {
	for (auto& constraint : physicsEngine->getContacts()) {
		CollisionHull* a = constraint.a;
		CollisionHull* b = constraint.b;

		Body* bodyA = constraint.bodyA;
		Body* bodyB = constraint.bodyB;

		// TODO: skip if both are asleep
		
		if (!bodyA->canCollideWith(*bodyB)) {
			continue;
		}

		// TODO: remove contact if not passing broadphase

		Manifold* manifold = &constraint.manifold;
		
		constraint.solve();
		computeBasis(manifold->normal, manifold->tangents,
				manifold->tangents + 1);

		// TODO: fire collision listener
	}
}

void Physics::ContactManager::addContact(CollisionHull& a, CollisionHull& b) {
	Physics::Body* bodyA = a.body;
	Physics::Body* bodyB = b.body;

	if (!bodyA->canCollideWith(*bodyB)) {
		return;
	}

	// TODO: check contact edges or something

	physicsEngine->getContacts().emplace_back(a, b);
	
	// TODO: awaken bodies
}

void Physics::ContactManager::clearContacts() {
	physicsEngine->getContacts().clear();
}

inline static void computeBasis(const Vector3f& a, Vector3f* b, Vector3f* c) {
	// Suppose vector a has all equal components and is a unit vector:
	// a = (s, s, s)
	// Then 3*s*s = 1, s = sqrt(1/3) = 0.57735027.
	// This means that at least one component of a
	// unit vector must be greater or equal to 0.57735027.
	// Can use SIMD select operation.

	if (a.x >= 0.57735027f || -a.x <= -0.57735027f) {
		*b = Vector3f(a.y, -a.x, 0.f);
	}
	else {
		*b = Vector3f(0.f, a.z, -a.y);
	}

	*b = Math::normalize(*b);
	*c = Math::cross(a, *b);
}

