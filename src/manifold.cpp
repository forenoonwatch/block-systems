#include "manifold.hpp"

#include "body.hpp"

void Physics::Manifold::addContact(const Vector3f& point, float penetration,
		const FeaturePair& fp) {
	Contact& cs = contacts[numContacts];
	cs.rA = point - hullA->getBody()->getWorldCenter();
	cs.rB = point - hullB->getBody()->getWorldCenter();
	cs.penetration = penetration;
	cs.fp = fp;

	++numContacts;
}

