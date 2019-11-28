#pragma once

#include "body.hpp"
#include "contact-manager.hpp"

#include <engine/core/array-list.hpp>

#include <engine/math/vector.hpp>

#include <engine/ecs/ecs-system.hpp>

class Game;

namespace Physics {
	constexpr const Vector3f GRAVITY = Vector3f(0.f, -9.81f, 0.f);

	class CollisionHull;

	class GravitySystem : public ECS::System {
		public:
			virtual void operator()(Game& game, float deltaTime) override;
	};

	class PhysicsEngine : public ECS::System {
		public:
			PhysicsEngine();

			Body* addBody();

			virtual void operator()(Game& game, float deltaTime) override;

			void addHull(Body& body, CollisionHull& hull);

			inline ArrayList<Body*>& getBodies();

			~PhysicsEngine();
		private:
			ContactManager contactManager;
			ArrayList<Body*> bodies;

			bool newHull;
	};
};

#include "physics.inl"

