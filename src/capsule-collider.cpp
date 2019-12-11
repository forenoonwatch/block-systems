#include "capsule-collider.hpp"

#include <engine/math/math.hpp>

Physics::CapsuleCollider::CapsuleCollider(const ColliderHints& hints)
		: Collider(hints)
		, points({hints.getP0(), hints.getP1()})
		, radius(hints.getRadius()) {
	Vector3f f = Math::normalize(points[1] - points[0]);
	Vector3f r, u;
	Math::computeBasis(f, r, u);
	
	Vector3f offset = (r + u + f) * radius;

	baseAABB = AABB(Math::min(points[0] - offset, points[1] + offset),
			Math::max(points[0] - offset, points[1] + offset));
}

AABB Physics::CapsuleCollider::computeAABB() const {
	return baseAABB.transform(worldTransform.toMatrix());	
}

void Physics::CapsuleCollider::calcMassData(float& mass, Matrix3f& inertia,
		Vector3f& centerOfMass) const {
	float r2 = radius * radius;
	float h = Math::length(points[1] - points[0]);
	float h2 = h * h;
	float rh = radius * h;

	// vSphere = 4/3 * pi * r^3
	float vSphere = 4.f * MATH_PI * r2 * radius / 3.f;
	// vCylinder = pi * r^2 * h
	float vCylinder = MATH_PI * r2 * h;

	mass = density * (vSphere + vCylinder);

	inertia = Matrix3f(mass);
	inertia[0][0] *= vCylinder * ( (h2 / 12.f) + (r2 / 4.f) )
			+ vSphere * ( (2.f * r2 / 5.f) + (h2 / 2.f) + (3.f * rh / 8.f) );
	inertia[1][1] *= vCylinder * (r2 / 2.f) + vSphere * (2.f * r2 / 5.f);
	inertia[2][2] = inertia[0][0];

	centerOfMass = localTransform.transform((points[0] + points[1]) * 0.5f,
			1.f);

	// TODO: add transform.toRotationMatrix();
	// TODO: probably excise this to body
	Matrix3f rL(localTransform.toMatrix());
	inertia = rL * inertia * Math::transpose(rL);
	inertia += (Matrix3f(Math::dot(centerOfMass, centerOfMass))
			- Math::outerProduct(centerOfMass, centerOfMass)) * mass;
}

