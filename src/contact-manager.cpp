#include "contact-manager.hpp"

#include "physics.hpp"
#include "body.hpp"
#include "collision-hull.hpp"

Physics::ContactManager::ContactManager(PhysicsEngine& physicsEngine)
		: physicsEngine(&physicsEngine) {

}

void Physics::ContactManager::findNewContacts() {
	for (uint32 i = 0; i < physicsEngine->getBodies().size(); ++i) {
		Body* a = physicsEngine->getBodies()[i];

		for (uint32 j = i + 1; j < physicsEngine->getBodies().size(); ++j) {
			Body* b = physicsEngine->getBodies()[j];

			addContact(*a->collisionHull, *b->collisionHull);
		}
	}
}

void Physics::ContactManager::testCollisions() {
	for (auto& constraint : contactList) {
		CollisionHull* hullA = constraint.hullA;
		CollisionHull* hullB = constraint.hullB;
		
		Body* bodyA = hullA->body;
		Body* bodyB = hullB->body;

		constraint.flags &= ~ContactConstraint::FLAG_ISLAND;

		// TODO: if neither body is awake, skip

		if (!bodyA->canCollideWith(*bodyB)) {
			removeContact(constraint);
			continue;
		}

		// TODO: remove contact if fails broadphase check

		//Manifold* manifold = &constraint.manifold;
		// TODO: old/new manifold thing

		constraint.testCollision();
	
		// TODO: warm starting stuff	
	}
}

void Physics::ContactManager::addContact(CollisionHull& a, CollisionHull& b) {
	Body* bodyA = a.body;
	Body* bodyB = b.body;

	if (!bodyA->canCollideWith(*bodyB)) {
		return;
	}

	for (ContactEdge* edge : bodyA->contactList) {
		if (edge->other == bodyB && (&a == edge->constraint->hullA)
				&& (&b == edge->constraint->hullB)) {
			return;
		}
	}

	contactList.emplace_back(a, b);

	// TODO: awaken A and B
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
	contactList[i].bodyA->removeEdge(&contactList[i].edgeA);
	contactList[i].bodyB->removeEdge(&contactList[i].edgeB);

	// TODO: wake up A and B
	
	contactList[i] = contactList.back();
	contactList.pop_back();
}

