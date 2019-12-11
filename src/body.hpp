#pragma once

#include <engine/core/array-list.hpp>

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/math/transform.hpp>

namespace Physics {
	class ContactEdge;
	class PhysicsEngine;

	class Collider;
	class ColliderHints;

	enum class BodyType {
		DYNAMIC,
		STATIC,
		KINEMATIC
	};

	struct BodyHints {
		BodyHints();

		BodyType type;

		bool active;
		bool awake;
		bool allowSleep;

		Vector3f velocity;
		Vector3f angularVelocity;

		Vector3f force;
		Vector3f torque;
	};

	class Body {
		public:
			void addCollider(const ColliderHints& hints);

			inline void applyForce(const Vector3f& force);
			inline void applyForce(const Vector3f& force,
					const Vector3f& worldPoint);

			inline void applyTorque(const Vector3f& torque);

			inline void applyImpulse(const Vector3f& impulse);
			inline void applyImpulse(const Vector3f& impulse,
					const Vector3f& worldPoint);

			inline bool canCollideWith(const Body& other) const;

			inline void setToAwake();
			inline void setToSleep();
			inline void setInIsland();
			inline void setNotInIsland();

			inline void setTransform(const Transform& transform);

			inline void setVelocity(const Vector3f& velocity);
			inline void setAngularVelocity(const Vector3f& angularVelocity);

			inline void setForce(const Vector3f& force);
			inline void setTorque(const Vector3f& torque);

			inline void setMass(float mass);

			inline void setInvInertiaLocal(const Matrix3f& invInertiaLocal);

			inline bool isAwake() const;
			inline bool isActive() const;
			inline bool canSleep() const;

			inline bool isDynamic() const;
			inline bool isStatic() const;
			inline bool isKinematic() const;

			inline bool isInIsland() const;

			inline Vector3f getVelocityAt(const Vector3f& worldPoint) const;
			
			inline const Transform& getTransform() const;

			inline Vector3f& getLocalCenter();
			inline const Vector3f& getLocalCenter() const;

			inline const Vector3f& getWorldCenter() const;

			inline Vector3f& getVelocity();
			inline const Vector3f& getVelocity() const;

			inline Vector3f& getAngularVelocity();
			inline const Vector3f& getAngularVelocity() const;

			inline Vector3f& getForce();
			inline const Vector3f& getForce() const;

			inline Vector3f& getTorque();
			inline const Vector3f& getTorque() const;

			inline float getMass() const;
			inline float getInvMass() const;

			inline Matrix3f& getInvInertiaLocal();
			inline const Matrix3f& getInvInertiaLocal() const;

			inline const Matrix3f& getInvInertiaWorld() const;

			inline const ArrayList<Collider*>& getColliders() const;

			~Body();
		private:
			enum Flags {
				FLAG_AWAKE			= 1,  // if the object is awake
				FLAG_ACTIVE			= 2,  // if the object can have calcs done
				FLAG_ALLOW_SLEEP	= 4,  // if sleep allowed
				FLAG_ISLAND			= 8,  // marker for island building
				FLAG_DYNAMIC		= 16, // collision, integration
				FLAG_STATIC			= 32, // collision, no integration
				FLAG_KINEMATIC		= 64  // no collision, integration
			};

			NULL_COPY_AND_ASSIGN(Body);

			Body(PhysicsEngine& physicsEngine, const BodyHints& hints);

			inline void removeEdge(ContactEdge* edge);

			void calcMassData();

			PhysicsEngine* physicsEngine;

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

			ArrayList<Collider*> colliders;

			ArrayList<ContactEdge*> contactList;

			float sleepTime;
			
			uint32 flags;

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

