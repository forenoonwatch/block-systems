#pragma once

#include "collider.hpp"

namespace Physics {
	class Manifold;
	
	typedef void (*CollisionCallback)(Manifold&, Collider&,
			Collider&);

	// Self Collisions
	void collisionSphereSphere(Manifold&, Collider&, Collider&);
	void collisionCapsuleCapsule(Manifold&, Collider&, Collider&);
	void collisionConvexConvex(Manifold&, Collider&, Collider&);
	
	// Sphere-Plane
	void collisionSpherePlane(Manifold&, Collider&, Collider&);
	void collisionPlaneSphere(Manifold&, Collider&, Collider&);

	// Sphere-Capsule
	void collisionSphereCapsule(Manifold&, Collider&, Collider&);
	void collisionCapsuleSphere(Manifold&, Collider&, Collider&);

	// Sphere-Convex
	void collisionSphereConvex(Manifold&, Collider&, Collider&);
	void collisionConvexSphere(Manifold&, Collider&, Collider&);

	// Plane-Capsule
	void collisionPlaneCapsule(Manifold&, Collider&, Collider&);
	void collisionCapsulePlane(Manifold&, Collider&, Collider&);

	// Plane-Convex
	void collisionPlaneConvex(Manifold&, Collider&, Collider&);
	void collisionConvexPlane(Manifold&, Collider&, Collider&);

	// Capsule-Convex
	void collisionCapsuleConvex(Manifold&, Collider&, Collider&);
	void collisionConvexCapsule(Manifold&, Collider&, Collider&);

	// TODO: replace this with a dispatch function
	constexpr static CollisionCallback COLLISION_DISPATCH[ColliderType::NUM_TYPES][ColliderType::NUM_TYPES] = {
		{ collisionSphereSphere, collisionSpherePlane, collisionSphereCapsule, collisionSphereConvex },
		{ collisionPlaneSphere, nullptr, collisionPlaneCapsule, collisionPlaneConvex },
		{ collisionCapsuleSphere, collisionCapsulePlane, collisionCapsuleCapsule, collisionCapsuleConvex },
		{ collisionConvexSphere, collisionConvexPlane, collisionConvexCapsule, collisionConvexConvex },
	};
};

