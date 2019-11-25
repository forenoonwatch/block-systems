#pragma once

#include <engine/math/vector.hpp>

namespace Physics {
	constexpr const uint32 MAX_CONTACTS = 1;

	class Body;

	struct ContactState {
		Vector3f rA;
		Vector3f rB;

		float penetration;

		float normalImpulse;
		float tangentImpulse[2];
		
		float bias;
		
		float normalMass;
		float tangentMass[2];
	};

	class Manifold {
		public:
			Manifold(Body& a, Body& b);

			bool testCollision();

			void preSolve(float deltaTime);
			void solve();

			inline void setNormal(const Vector3f& n) { normal = n; }

			void addContact(const Vector3f& point, float penetration);

			inline Body* getBodyA() { return bodyA; }
			inline Body* getBodyB() { return bodyB; }
		private:
			Body* bodyA;
			Body* bodyB;

			Vector3f normal;
			Vector3f tangents[2];

			ContactState contacts[MAX_CONTACTS];
			uint32 numContacts;

			float friction;
			float restitution;
			
			void correctInfiniteMass();

			float mixFriction();
			float mixRestitution();
	};
};

