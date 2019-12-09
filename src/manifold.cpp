#include "manifold.hpp"

#include "body.hpp"

void Physics::Manifold::addContact(const Vector3f& point, float penetration,
		const FeaturePair& fp) {
	Contact& cs = contacts[numContacts];
	cs.rA = point - hullA->getBody()->getTransform().getPosition();
	cs.rB = point - hullB->getBody()->getTransform().getPosition();
	cs.penetration = penetration;
	cs.fp = fp;

	++numContacts;
}

