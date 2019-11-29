#include "manifold.hpp"

#include "body.hpp"

void Physics::Manifold::addContact(const Vector3f& point, float penetration) {
	Contact& cs = contacts[numContacts];
	cs.rA = point - hullA->body->getTransform().getPosition();
	cs.rB = point - hullB->body->getTransform().getPosition();
	cs.penetration = penetration;
	cs.fp.key = 0;

	++numContacts;
}

