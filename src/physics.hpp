#pragma once

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>

class Game;

namespace Physics {
	struct Body {
		Vector3f velocity;
		Vector3f angularVelocity;

		Vector3f force;
		Vector3f torque;

		float mass;
		float invMass;

		Matrix3f invInertiaLocal;
		Matrix3f invInertiaWorld;
	};

	inline void applyForce(Body& body, const Vector3f& force);
	inline void applyForce(Body& body, const Vector3f& force,
			const Vector3f& localPoint);

	inline void applyTorque(Body& body, const Vector3f& torque);

	inline void applyImpulse(Body& body, const Vector3f& impulse);
	inline void applyImpulse(Body& body, const Vector3f& impulse,
			const Vector3f& localPoint);

	void integrateVelocities(Game& game, float deltaTime);
};

#include "physics.inl"

