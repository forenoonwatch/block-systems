#pragma once

#include <engine/core/common.hpp>

#include <engine/ecs/ecs-system.hpp>

namespace Physics {
	class PhysicsEngine;

	class ContactFinder : public ECS::System {
		public:
			ContactFinder(PhysicsEngine& physicsEngine);

			virtual void operator()(Game& game, float deltaTime) override;
		private:
			PhysicsEngine* physicsEngine;
	};
};

