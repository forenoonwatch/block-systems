#pragma once

#include <engine/core/array-list.hpp>

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/math/transform.hpp>

namespace Physics {
	class CollisionHull;
	class ContactEdge;

	class Body {
		public:
			enum Flags {
				FLAG_STATIC = 1
			};

			inline void applyForce(const Vector3f& force);
			inline void applyForce(const Vector3f& force,
					const Vector3f& worldPoint);

			inline void applyTorque(const Vector3f& torque);

			inline void applyImpulse(const Vector3f& impulse);
			inline void applyImpulse(const Vector3f& impulse,
					const Vector3f& worldPoint);

			inline Vector3f getVelocityAt(const Vector3f& worldPoint) const;

			inline bool canCollideWith(const Body& other) const;

			Transform transform;

			Vector3f localCenter;
			Vector3f worldCenter;

			Vector3f velocity;
			Vector3f angularVelocity;

			Vector3f force;
			Vector3f torque;

			float mass;
			float invMass;

			Matrix3f invInertiaLocal;
			Matrix3f invInertiaWorld;

			CollisionHull* collisionHull;

			uint32 index;
			uint32 flags;
		private:
			friend class ContactManager;
			friend class ContactConstraint;
	};

	struct BodyHandle {
		Body* body;
	};
};

#include "body.inl"

