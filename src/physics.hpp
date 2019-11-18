#pragma once

#include <engine/math/matrix.hpp>
#include <engine/math/quaternion.hpp>

#include <engine/ecs/ecs-system.hpp>

class Game;

namespace Physics {
	constexpr const Vector3f GRAVITY = Vector3f(0.f, -9.81f, 0.f);

	class Body {
		public:
			inline void applyForce(const Vector3f& force);
			inline void applyForce(const Vector3f& force,
					const Vector3f& worldPoint);

			inline void applyTorque(const Vector3f& torque);

			inline void applyImpulse(const Vector3f& impulse);
			inline void applyImpulse(const Vector3f& impulse,
					const Vector3f& worldPoint);

			inline Vector3f getVelocityAt(const Vector3f& worldPoint) const;

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

	class GravitySystem : public ECS::System {
		public:
			virtual void operator()(Game& game, float deltaTime) override;
	};

	class IntegrateVelocities : public ECS::System {
		public:
			virtual void operator()(Game& game, float deltaTime) override;
	};
};

#include "physics.inl"

