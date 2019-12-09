#pragma once

#include <engine/core/common.hpp>

#include <engine/math/aabb.hpp>
#include <engine/math/transform.hpp>

namespace Physics {
	class Body;

	class CollisionHull {
		public:
			enum HullType {
				TYPE_SPHERE = 0,
				TYPE_PLANE,
				TYPE_CONVEX_HULL,
				NUM_TYPES
			};

			inline CollisionHull(enum HullType type)
					: type(type)
					, friction(0.f)
					, restitution(0.f)
					, body(nullptr) {}

			virtual AABB computeAABB(const Transform& tf) const = 0;

			inline void setFriction(float friction) {
				this->friction = friction;
			}

			inline void setRestitution(float restitution) {
				this->restitution = restitution;
			}

			inline float getFriction() const { return friction; }
			inline float getRestitution() const { return restitution; }

			inline Body* getBody() { return body; }
			inline const Body* getBody() const { return body; }

			inline enum HullType getType() const { return type; }

			virtual ~CollisionHull() {}
		protected:
			float friction;
			float restitution;

			Body* body;

			const enum HullType type;

			friend class Body;
		private:
			int32 broadphaseIndex;

			friend class Broadphase;
			friend class ContactManager;
	};
};

