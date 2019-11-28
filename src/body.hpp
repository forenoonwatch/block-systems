#pragma once

#include <engine/core/array-list.hpp>

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/math/transform.hpp>

namespace Physics {
	class ContactEdge;
	class PhysicsEngine;
	class CollisionHull;

	class Body {
		public:
			enum Flags {
				FLAG_AWAKE			= 1,  // if the object is awake
				FLAG_ACTIVE			= 2,  // if the object can have calcs done
				FLAG_ALLOW_SLEEP	= 4,  // if sleep allowed
				FLAG_ISLAND			= 8,  // marker for island building
				FLAG_DYNAMIC		= 16, // collision, integration
				FLAG_STATIC			= 32, // collision, no integration
				FLAG_KINEMATIC		= 64  // no collision, integration
			};
			
			inline Body(PhysicsEngine& physicsEngine)
					: transform()
					, localCenter()
					, worldCenter()
					, velocity()
					, angularVelocity()
					, mass(0.f)
					, invMass(0.f)
					, invInertiaLocal(0.f)
					, invInertiaWorld(0.f)
					, force()
					, torque()
					, physicsEngine(&physicsEngine)
					, sleepTime(0.f)
					, flags(0) {}

			void setCollisionHull(CollisionHull* hull);

			inline void applyForce(const Vector3f& force);
			inline void applyForce(const Vector3f& force,
					const Vector3f& worldPoint);

			inline void applyTorque(const Vector3f& torque);

			inline void applyImpulse(const Vector3f& impulse);
			inline void applyImpulse(const Vector3f& impulse,
					const Vector3f& worldPoint);

			inline Vector3f getVelocityAt(const Vector3f& worldPoint) const;

			inline bool canCollideWith(const Body& other) const;

			inline void setToAwake();
			inline void setToSleep();
			inline void setInIsland();

			inline bool isAwake() const;
			inline bool isActive() const;
			inline bool canSleep() const;

			inline bool isDynamic() const;
			inline bool isStatic() const;
			inline bool isKinematic() const;

			inline bool isInIsland() const;

			inline CollisionHull* getCollisionHull();

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

			uint32 flags;
		private:
			NULL_COPY_AND_ASSIGN(Body);

			PhysicsEngine* physicsEngine;

			CollisionHull* collisionHull;

			ArrayList<ContactEdge*> contactList;
			float sleepTime;

			inline void removeEdge(ContactEdge* edge);

			friend class PhysicsEngine;
			friend class Island;
			friend class ContactManager;
			friend class ContactConstraint;
	};

	struct BodyHandle {
		inline explicit BodyHandle(Body* body)
				: body(body) {}

		Body* body;
	};
};

#include "body.inl"

