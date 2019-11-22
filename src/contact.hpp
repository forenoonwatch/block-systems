#pragma once

#include <engine/core/common.hpp>

#include <engine/math/matrix.hpp>

namespace Physics {
	constexpr const uint32 MAX_CONTACTS = 8;

	class Body;
	class CollisionHull;
	class ContactConstraint;

	struct Contact {
		Vector3f position; // world position of contact
		
		float penetration;
		
		float normalImpulse;
		float tangentImpulse[2];
		
		float bias; // restitution + Baumgarte
		
		float normalMass;
		float tangentMass[2];
	};

	struct Manifold {
		CollisionHull* a;
		CollisionHull* b;

		Vector3f normal; // A -> B
		Vector3f tangents[2];	// Tangent vectors
		
		Contact contacts[Physics::MAX_CONTACTS];
		uint32 numContacts;

		//bool sensor;
		
		inline void setPair(CollisionHull& a, CollisionHull& b);
	};

	class ContactConstraint {
		public:
			enum Flags {
				COLLIDING = 1,
				WAS_COLLIDING = 2
			};

			ContactConstraint(CollisionHull& a, CollisionHull& b);

			void solve();
		private:
			CollisionHull* a;
			CollisionHull* b;

			Body* bodyA;
			Body* bodyB;

			float friction;
			float restitution;

			Manifold manifold;

			uint32 flags;

			friend class ContactManager;
			friend class ContactSolver;
			friend class PhysicsEngine;
	};
};

inline void Physics::Manifold::setPair(CollisionHull& a, CollisionHull& b) {
	this->a = &a;
	this->b = &b;

	//sensor = a->sensor || b->sensor;
}

