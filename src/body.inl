
inline void Physics::Body::applyForce(const Vector3f& force) {
	this->force += force * mass;
}

inline void Physics::Body::applyForce(const Vector3f& force,
		const Vector3f& worldPoint) {
	this->force += force * mass;
	this->torque += Math::cross(worldPoint - worldCenter, force);
}

inline void Physics::Body::applyTorque(const Vector3f& torque) {
	this->torque += torque;
}

inline void Physics::Body::applyImpulse(const Vector3f& impulse) {
	velocity += impulse * invMass;
}

inline void Physics::Body::applyImpulse(const Vector3f& impulse,
		const Vector3f& worldPoint) {
	velocity += impulse * invMass;
	angularVelocity += invInertiaWorld
		* Math::cross(worldPoint - worldCenter, impulse);
}

inline Vector3f Physics::Body::getVelocityAt(
		const Vector3f& worldPoint) const {
	return velocity + Math::cross(angularVelocity, worldPoint - worldCenter);
}

inline bool Physics::Body::canCollideWith(const Body& other) const {
	if (this == &other) {
		return false;
	}

	if (!isDynamic() && !other.isDynamic()) {
		return false;
	}

	// TODO: layers: aka collision groups

	return true;
}

inline void Physics::Body::removeEdge(Physics::ContactEdge* edge) {
	for (uint32 i = 0; i < contactList.size(); ++i) {
		if (contactList[i] == edge) {
			contactList[i] = contactList.back();
			contactList.pop_back();

			return;
		}
	}
}

inline void Physics::Body::setToAwake() {
	flags |= Physics::Body::FLAG_AWAKE;
}

inline void Physics::Body::setToSleep() {
	flags &= ~Physics::Body::FLAG_AWAKE;
}

inline void Physics::Body::setInIsland() {
	flags |= Physics::Body::FLAG_ISLAND;
}

inline bool Physics::Body::isAwake() const {
	return flags & Physics::Body::FLAG_AWAKE;
}

inline bool Physics::Body::isActive() const {
	return flags & Physics::Body::FLAG_ACTIVE;
}

inline bool Physics::Body::canSleep() const {
	return flags & Physics::Body::FLAG_ALLOW_SLEEP;
}

inline bool Physics::Body::isDynamic() const {
	return flags & Physics::Body::FLAG_DYNAMIC;
}

inline bool Physics::Body::isStatic() const {
	return flags & Physics::Body::FLAG_STATIC;
}

inline bool Physics::Body::isKinematic() const {
	return flags & Physics::Body::FLAG_KINEMATIC;
}

inline bool Physics::Body::isInIsland() const {
	return flags & Physics::Body::FLAG_ISLAND;
}

