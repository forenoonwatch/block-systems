
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
	velocity += impulse;
}

inline void Physics::Body::applyImpulse(const Vector3f& impulse,
		const Vector3f& worldPoint) {
	velocity += impulse;
	angularVelocity += invInertiaWorld
		* Math::cross(worldPoint - worldCenter, impulse);
}

