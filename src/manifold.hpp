#pragma once

#include "contact.hpp"
#include "collider.hpp"

namespace Physics {
	class Body;

	class Manifold {
		public:
			inline Manifold();

			inline void setPair(Collider& a, Collider& b);

			void addContact(const Vector3f& point, float penetration,
					const FeaturePair& fp);

			inline void setNormal(const Vector3f& n) { normal = n; }

			inline Body* getBodyA() { return hullA->getBody(); }
			inline Body* getBodyB() { return hullB->getBody(); }

			inline const Vector3f& getNormal() const { return normal; }
		private:
			Collider* hullA;
			Collider* hullB;

			Vector3f normal;
			Vector3f tangents[2];

			Contact contacts[MAX_CONTACTS];
			uint32 numContacts;

			// TODO: bool sensor;

			friend class ContactConstraint;
			friend class ContactManager;
	};
};

inline Physics::Manifold::Manifold()
		: numContacts(0) {}

inline void Physics::Manifold::setPair(Collider& a, Collider& b) {
	hullA = &a;
	hullB = &b;

	// TODO: sensor = a.sensor || b.sensor;
}

