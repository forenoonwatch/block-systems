#include "contact.hpp"

#include "body.hpp"
#include "collision-hull.hpp"
#include "collision.hpp"

#include <engine/math/math.hpp>

static inline float mixRestitution(const Physics::CollisionHull& a,
		const Physics::CollisionHull& b) {
	return Math::max(a.restitution, b.restitution);
}

static inline float mixFriction(const Physics::CollisionHull& a,
		const Physics::CollisionHull& b) {
	return Math::sqrt(a.friction * b.friction);
}

Physics::ContactConstraint::ContactConstraint(Physics::CollisionHull& aIn,
			Physics::CollisionHull& bIn)
		: a(&aIn)
		, b(&bIn)
		, bodyA(aIn.body)
		, bodyB(bIn.body)
		, friction(mixFriction(aIn, bIn))
		, restitution(mixRestitution(aIn, bIn))
		, flags(0) {
	manifold.setPair(aIn, bIn);
	manifold.numContacts = 0;
}

void Physics::ContactConstraint::solve() {
	using Physics::ContactConstraint;

	manifold.numContacts = 0;

	Physics::COLLISION_DISPATCH[a->type][b->type](manifold, *a, *b);
	
	if (manifold.numContacts > 0) {
		if (flags & COLLIDING) {
			flags |= WAS_COLLIDING;
		}
		else {
			flags |= COLLIDING;
		}
	}
	else {
		if (flags & COLLIDING) {
			flags &= COLLIDING;
			flags |= WAS_COLLIDING;
		}
		else {
			flags &= ~WAS_COLLIDING;
		}
	}
}

