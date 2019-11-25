#pragma once

#include "collision-hull.hpp"

namespace Physics {
	class Manifold;
	
	typedef void (*CollisionCallback)(Manifold&, CollisionHull&,
			CollisionHull&);

	void collisionSphereSphere(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexConvex(Manifold&, CollisionHull&, CollisionHull&);
	
	void collisionSphereConvex(Manifold&, CollisionHull&, CollisionHull&);
	void collisionConvexSphere(Manifold&, CollisionHull&, CollisionHull&);

	static CollisionCallback COLLISION_DISPATCH[CollisionHull::NUM_TYPES][CollisionHull::NUM_TYPES] = {
		{ collisionSphereSphere, collisionSphereConvex },
		{ collisionConvexSphere, collisionConvexConvex }
	};
};

