#include "contact-solver.hpp"

#include "contact.hpp"
#include "contact-constraint.hpp"
#include "contact-manager.hpp"

Physics::ContactSolver::ContactSolver(ContactManager& contactManager)
		: contactManager(&contactManager) {}

void Physics::ContactSolver::preSolve(float deltaTime) {
	for (auto& contact : contactManager->contactList) {
		contact.preSolve(deltaTime);
	}
}

void Physics::ContactSolver::solve() {
	for (auto& contact : contactManager->contactList) {
		contact.solve();
	}
}

