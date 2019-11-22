#pragma once


#include <engine/core/array-list.hpp>

#include "contact-state.hpp"

namespace Physics {
	class PhysicsEngine;

	class ContactSolver {
		public:
			ContactSolver(Physics::PhysicsEngine&);

			void preSolve(float deltaTime);
			void solve();

			void flush();
		private:
			Physics::PhysicsEngine* physicsEngine;

			bool frictionEnabled;
	};
};

