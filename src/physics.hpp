#pragma once

#include "body.hpp"
#include "contact.hpp"
#include "contact-state.hpp"
#include "contact-manager.hpp"

#include <engine/core/array-list.hpp>

#include <engine/math/vector.hpp>

#include <engine/ecs/ecs-system.hpp>

class Game;

namespace Physics {
	constexpr const Vector3f GRAVITY = Vector3f(0.f, -9.81f, 0.f);

	struct VelocityState {
		inline VelocityState(const Vector3f& v, const Vector3f& w)
				: v(v)
				, w(w) {}
		
		Vector3f v;
		Vector3f w;
	};

	class GravitySystem : public ECS::System {
		public:
			virtual void operator()(Game& game, float deltaTime) override;
	};

	class PhysicsEngine : public ECS::System {
		public:
			PhysicsEngine();

			Body* addBody();

			virtual void operator()(Game& game, float deltaTime) override;

			inline ContactManager& getContactManager();

			inline ArrayList<Body>& getBodies();
			inline ArrayList<VelocityState>& getVelocityStates();
			inline ArrayList<ContactConstraint>& getContacts();
			inline ArrayList<ContactConstraintState>& getContactStates();
		private:
			ContactManager contactManager;

			ArrayList<Body> bodies;
			ArrayList<VelocityState> velocityStates;
			ArrayList<ContactConstraint> contacts;
			ArrayList<ContactConstraintState> contactStates;

			void initConstraintStates();
	};
};

#include "physics.inl"

