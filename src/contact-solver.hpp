#pragma once

#include <engine/core/common.hpp>

namespace Physics {
	class ContactManager;

	class ContactSolver {
		public:
			ContactSolver(ContactManager& contactManager);

			void preSolve(float deltaTime);
			void solve();
		private:
			NULL_COPY_AND_ASSIGN(ContactSolver);

			ContactManager* contactManager;
	};
};

