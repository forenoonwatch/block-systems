#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class Manifold;
	
	typedef void (*CollisionCallback)(Manifold&, CollisionHull&,
			CollisionHull&);

	// Self Collisions
	void collisionSphereSphere(Manifold&, CollisionHull&, CollisionHull&);
	void collisionCapsuleCapsule(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexConvex(Manifold&, CollisionHull&, CollisionHull&);
	
	// Sphere-Plane
	void collisionSpherePlane(Manifold&, CollisionHull&, CollisionHull&);
	void collisionPlaneSphere(Manifold&, CollisionHull&, CollisionHull&);

	// Sphere-Capsule
	void collisionSphereCapsule(Manifold&, CollisionHull&, CollisionHull&);
	void collisionCapsuleSphere(Manifold&, CollisionHull&, CollisionHull&);

	// Sphere-Convex
	void collisionSphereConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexSphere(Manifold&, CollisionHull&, CollisionHull&);

	// Plane-Capsule
	void collisionPlaneCapsule(Manifold&, CollisionHull&, CollisionHull&);
	void collisionCapsulePlane(Manifold&, CollisionHull&, CollisionHull&);

	// Plane-Convex
	void collisionPlaneConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexPlane(Manifold&, CollisionHull&, CollisionHull&);

	// Capsule-Convex
	void collisionCapsuleConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexCapsule(Manifold&, CollisionHull&, CollisionHull&);

	// TODO: replace this with a dispatch function
	constexpr static CollisionCallback COLLISION_DISPATCH[CollisionHull::NUM_TYPES][CollisionHull::NUM_TYPES] = {
		{ collisionSphereSphere, collisionSpherePlane, collisionSphereCapsule, collisionSphereConvex },
		{ collisionPlaneSphere, nullptr, collisionPlaneCapsule, collisionPlaneConvex },
		{ collisionCapsuleSphere, collisionCapsulePlane, collisionCapsuleCapsule, collisionCapsuleConvex },
		{ collisionConvexSphere, collisionConvexPlane, collisionConvexCapsule, collisionConvexConvex },
	};
};

