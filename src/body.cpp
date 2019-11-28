#include "body.hpp"

#include "collision-hull.hpp"
#include "physics.hpp"

void Physics::Body::setCollisionHull(CollisionHull* hull) {
	collisionHull = hull;
	hull->body = this;

	physicsEngine->addHull(*this, *hull);
}

