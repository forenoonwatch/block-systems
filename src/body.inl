
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

	return true;
}

