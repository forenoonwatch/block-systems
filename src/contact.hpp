#pragma once

#include <engine/math/vector.hpp>

namespace Physics {
	constexpr const uint32 MAX_CONTACTS = 8; // TODO: do not limit contacts

	class Body;
	class ContactConstraint;
	class Manifold;

	union FeaturePair { // TODO: potentially swap to uint64 and 4 uint16s 
		struct {
			uint8 inR;
			uint8 outR;
			uint8 inI;
			uint8 outI;
		};

		uint32 key;
	};

	struct Contact {
		Vector3f rA;
		Vector3f rB;

		float penetration;

		float normalImpulse;
		float tangentImpulse[2];

		float bias;

		float normalMass;
		float tangentMass[2];

		FeaturePair fp;
	};

	struct ContactEdge {
		Body* other;
		ContactConstraint* constraint;
	};
};

