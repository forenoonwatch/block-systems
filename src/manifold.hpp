#pragma once

#include "contact.hpp"
#include "collision-hull.hpp"

namespace Physics {
	class Body;

	class Manifold {
		public:
			inline Manifold();

			inline void setPair(CollisionHull& a, CollisionHull& b);

			void addContact(const Vector3f& point, float penetration);

			inline void setNormal(const Vector3f& n) { normal = n; }

			inline Body* getBodyA() { return hullA->body; }
			inline Body* getBodyB() { return hullB->body; }
		private:
			CollisionHull* hullA;
			CollisionHull* hullB;

			Vector3f normal;
			Vector3f tangents[2];

			Contact contacts[MAX_CONTACTS];
			uint32 numContacts;

			// bool sensor;

			friend class ContactConstraint;
	};
};

inline Physics::Manifold::Manifold()
		: numContacts(0) {}

inline void Physics::Manifold::setPair(CollisionHull& a, CollisionHull& b) {
	hullA = &a;
	hullB = &b;

	// sensor = a.sensor || b.sensor;
}

