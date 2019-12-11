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

			ContactConstraint(Collider& a, Collider& b);

			void preSolve(float deltaTime);
			void solve();

			void testCollision();

			inline void setInIsland() { flags |= FLAG_ISLAND; }

			inline bool isInIsland() const { return flags & FLAG_ISLAND; }
			inline bool isColliding() const { return flags & FLAG_COLLIDING; }
		private:
			Collider* colliderA;
			Collider* colliderB;

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

