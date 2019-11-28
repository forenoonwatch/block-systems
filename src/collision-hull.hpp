#pragma once

#include <engine/core/common.hpp>

namespace Physics {
	class Body;

	class CollisionHull {
		public:
			enum HullType {
				TYPE_SPHERE = 0,
				TYPE_CONVEX_HULL,
				NUM_TYPES
			};

			inline CollisionHull(enum HullType type)
					: type(type)
					, friction(0.f)
					, restitution(0.f)
					, body(nullptr) {}

			virtual ~CollisionHull() {}

			float friction;
			float restitution;

			Body* body;

			const enum HullType type;
		private:
			int32 broadphaseIndex;

			friend class Broadphase;
	};
};

