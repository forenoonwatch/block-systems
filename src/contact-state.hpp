#pragma once

#include <engine/core/common.hpp>
#include <engine/math/matrix.hpp>

#include "contact.hpp"

namespace Physics {
	struct ContactState {
		Vector3f rA; // vector from COM to contact point
		Vector3f rB;

		float penetration;

		float normalImpulse;
		float tangentImpulse[2];

		float bias;

		float normalMass;
		float tangentMass[2];
	};

	struct ContactConstraintState {
		ContactState contacts[Physics::MAX_CONTACTS];
		uint32 numContacts;
		
		Vector3f normal; // A -> B
		Vector3f tangents[2];

		Vector3f centerA;
		Vector3f centerB;
		
		Matrix3f iA;
		Matrix3f iB;
		
		float mA;
		float mB;
		
		float restitution;
		float friction;
		
		uint32 indexA;
		uint32 indexB;
	};
};

