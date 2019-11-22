#pragma once

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
					: type(type) {}

			float friction;
			float restitution;

			Body* body;

			const enum HullType type;
		private:
	};
};

