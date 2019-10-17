#pragma once

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>

class Game;

namespace Physics {
	constexpr const Vector3f GRAVITY = Vector3f(0.f, -9.81f, 0.f);

	struct Body {
		Vector3f localCenter;
		Vector3f worldCenter;

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
			const Vector3f& worldPoint);

	inline void applyTorque(Body& body, const Vector3f& torque);

	inline void applyImpulse(Body& body, const Vector3f& impulse);
	inline void applyImpulse(Body& body, const Vector3f& impulse,
			const Vector3f& worldPoint);

	void gravitySystem(Game& game, float deltaTime);
	void integrateVelocities(Game& game, float deltaTime);
};

#include "physics.inl"

