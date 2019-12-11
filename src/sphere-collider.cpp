#include "sphere-collider.hpp"

#include <engine/math/math.hpp>

AABB Physics::SphereCollider::computeAABB() const {
	return AABB(worldTransform.getPosition() - Vector3f(radius),
			worldTransform.getPosition() + Vector3f(radius));
}

void Physics::SphereCollider::calcMassData(float& mass, Matrix3f& inertia,
		Vector3f& centerOfMass) const {
	// V = 4/3 * pi * r^3
	mass = density * 4.f * MATH_PI * radius * radius * radius / 3.f;
	// I = diag(2/5 * MR^2)
	centerOfMass = localTransform.getPosition();

	// TODO: add transform.toRotationMatrix();
	// TODO: probably excise this to body
	Matrix3f rL(localTransform.toMatrix());
	inertia = Matrix3f(0.4f * mass * radius * radius);
	inertia = rL * inertia * Math::transpose(rL);
	inertia += (Matrix3f(Math::dot(centerOfMass, centerOfMass))
			- Math::outerProduct(centerOfMass, centerOfMass)) * mass;
}

