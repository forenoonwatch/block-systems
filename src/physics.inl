
inline Physics::ContactManager& Physics::PhysicsEngine::getContactManager() {
	return contactManager;
}

inline ArrayList<Physics::Body>& Physics::PhysicsEngine::getBodies() {
	return bodies;
}

inline ArrayList<Physics::VelocityState>&
		Physics::PhysicsEngine::getVelocityStates() {
	return velocityStates;
}

inline ArrayList<Physics::ContactConstraint>&
		Physics::PhysicsEngine::getContacts() {
	return contacts;
}

inline ArrayList<Physics::ContactConstraintState>&
		Physics::PhysicsEngine::getContactStates() {
	return contactStates;
}

