#pragma once

#include <engine/core/common.hpp>

#include <engine/math/aabb.hpp>
#include <engine/math/transform.hpp>

class IndexedModel;

namespace Physics {
	class Body;

	enum ColliderType {
		TYPE_SPHERE = 0,
		TYPE_PLANE,
		TYPE_CAPSULE,
		TYPE_CONVEX_HULL,

		NUM_TYPES,
		TYPE_INVALID
	};

	class ColliderHints {
		public:
			inline ColliderHints();

			void initSphere(float radius);
			void initPlane();
			void initCapsule(const Vector3f& p0, const Vector3f& p1,
					float radius);
			void initConvexHull(const IndexedModel& model);

			inline void setTransform(const Transform& transform);

			inline void setDensity(float density);

			inline void setFriction(float friction);
			inline void setRestitution(float restitution);

			inline float getRadius() const;

			inline const Vector3f& getP0() const;
			inline const Vector3f& getP1() const;

			inline const IndexedModel* getModel() const;
		private:
			enum ColliderType type;
			
			Transform transform;

			float density;

			float friction;
			float restitution;

			float radius;
			Vector3f p0;
			Vector3f p1;
			const IndexedModel* model;

			friend class Collider;
	};

	class Collider {
		public:
			static Collider* make(const ColliderHints& hints);

			virtual AABB computeAABB() const = 0;

			virtual void calcMassData(float& mass, Matrix3f& inertia,
					Vector3f& centerOfMass) const = 0;

			inline void setFriction(float friction);
			inline void setRestitution(float restitution);

			inline void setWorldTransform(const Transform& transform);

			inline const Transform& getLocalTransform() const;
			inline const Transform& getWorldTransform() const;

			inline float getDensity() const;

			inline float getFriction() const;
			inline float getRestitution() const;

			inline Body* getBody();
			inline const Body* getBody() const;

			inline enum ColliderType getType() const;

			virtual ~Collider() {}
		protected:
			inline Collider(const ColliderHints& hints)
					: type(hints.type)
					, localTransform(hints.transform)
					, worldTransform(hints.transform)
					, density(hints.density)
					, friction(hints.friction)
					, restitution(hints.restitution)
					, body(nullptr) {}

			const enum ColliderType type;
			
			Transform localTransform;
			Transform worldTransform;
			
			float density;

			float friction;
			float restitution;

			Body* body;

			friend class Body;
		private:
			int32 broadphaseIndex;

			friend class Broadphase;
			friend class ContactManager;
	};
};

#include "collider.inl"

