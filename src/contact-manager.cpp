#include "contact-manager.hpp"

#include "physics.hpp"
#include "body.hpp"
#include "collider.hpp"

Physics::ContactManager::ContactManager(PhysicsEngine& physicsEngine)
		: physicsEngine(&physicsEngine)
		, broadphase(*this) {}

void Physics::ContactManager::findNewContacts() {
	broadphase.updatePairs();

	// TODO: make sure to check this for multi-collider bodies	
	for (Body* body : physicsEngine->getBodies()) {
		body->transform.setPosition(body->worldCenter
				- body->transform.transform(body->localCenter, 0.f));

		for (Collider* collider : body->colliders) {
			broadphase.update(collider->broadphaseIndex,
					collider->computeAABB());
		}
	}
}

void Physics::ContactManager::testCollisions() {
	for (uint32 i = 0; i < contactList.size(); ++i) {
		ContactConstraint& constraint = contactList[i];

		Collider* collA = constraint.colliderA;
		Collider* collB = constraint.colliderB;
		
		Body* bodyA = collA->body;
		Body* bodyB = collB->body;

		constraint.flags &= ~ContactConstraint::FLAG_ISLAND;

		if (!bodyA->isAwake() && !bodyB->isAwake()) {
			continue;
		}

		if (!bodyA->canCollideWith(*bodyB)) {
			removeContact(i);
			--i;
			continue;
		}

		if (!broadphase.testOverlap(collA->broadphaseIndex,
				collB->broadphaseIndex)) {
			removeContact(i);
			--i;
			continue;
		}

		Manifold* manifold = &constraint.manifold;
		Manifold oldManifold = constraint.manifold;

		Vector3f ot0 = oldManifold.tangents[0];
		Vector3f ot1 = oldManifold.tangents[1];

		constraint.testCollision();

		for (uint32 i = 0; i < manifold->numContacts; ++i) {
			Contact& c = manifold->contacts[i];
			c.tangentImpulse[0] = c.tangentImpulse[1] = c.normalImpulse = 0.f;

			for (uint32 j = 0; j < oldManifold.numContacts; ++j) {
				Contact& oc = oldManifold.contacts[j];

				if (c.fp.key == oc.fp.key) {
					c.normalImpulse = oc.normalImpulse;

					Vector3f friction = ot0 * oc.tangentImpulse[0]
							+ ot1 * oc.tangentImpulse[1];

					c.tangentImpulse[0] = Math::dot(friction,
							manifold->tangents[0]);
					c.tangentImpulse[1] = Math::dot(friction,
							manifold->tangents[1]);

					break;
				}
			}
		}
	
		// TODO: fire contact listeners if any
	}
}

void Physics::ContactManager::addContact(Collider& a, Collider& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	if (!bodyA->canCollideWith(*bodyB)) {
		return;
	}

	for (ContactEdge* edge : bodyA->contactList) {
		if (edge->other == bodyB && (&a == edge->constraint->colliderA)
				&& (&b == edge->constraint->colliderB)) {
			return;
		}
	}

	contactList.emplace_back(a, b);

	bodyA->setToAwake();
	bodyB->setToAwake();
}

void Physics::ContactManager::removeContact(ContactConstraint& constraint) {
	for (uint32 i = 0; i < contactList.size(); ++i) {
		if (&contactList[i] == &constraint) {
			removeContact(i);
			return;
		}
	}
}

void Physics::ContactManager::removeContact(uint32 i) {
	contactList[i].bodyA->removeEdge(contactList[i].edgeA.get());
	contactList[i].bodyB->removeEdge(contactList[i].edgeB.get());

	contactList[i].bodyA->setToAwake();
	contactList[i].bodyB->setToAwake();
	
	contactList[i] = contactList.back();
	contactList.pop_back();
}

