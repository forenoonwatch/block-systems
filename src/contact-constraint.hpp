#pragma once

#include "contact.hpp"
#include "manifold.hpp"

namespace Physics {
	class ContactConstraint {
		public:
			enum {
				FLAG_COLLIDING		= 1,
				FLAG_WAS_COLLIDING	= 2,
				FLAG_ISLAND			= 4
			};

			ContactConstraint(CollisionHull& a, CollisionHull& b);

			void preSolve(float deltaTime);
			void solve();

			void testCollision();
		private:
			CollisionHull* hullA;
			CollisionHull* hullB;

			Body* bodyA;
			Body* bodyB;

			ContactEdge edgeA;
			ContactEdge edgeB;

			float friction;
			float restitution;

			Manifold manifold;

			uint32 flags;

			float mixFriction();
			float mixRestitution();

			friend class ContactManager;
	};
};

