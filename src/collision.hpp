#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class Manifold;
	
	typedef void (*CollisionCallback)(Manifold&, CollisionHull&,
			CollisionHull&);

	void collisionSphereSphere(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexConvex(Manifold&, CollisionHull&, CollisionHull&);
	
	void collisionSpherePlane(Manifold&, CollisionHull&, CollisionHull&);
	void collisionPlaneSphere(Manifold&, CollisionHull&, CollisionHull&);

	void collisionSphereConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexSphere(Manifold&, CollisionHull&, CollisionHull&);

	void collisionPlaneConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexPlane(Manifold&, CollisionHull&, CollisionHull&);

	static CollisionCallback COLLISION_DISPATCH[CollisionHull::NUM_TYPES]
			[CollisionHull::NUM_TYPES] = {
		{ collisionSphereSphere, collisionSpherePlane, collisionSphereConvex },
		{ collisionPlaneSphere, nullptr, collisionPlaneConvex },
		{ collisionConvexSphere, collisionConvexPlane, collisionConvexConvex },
	};
};

