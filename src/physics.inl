
inline void Physics::applyForce(Physics::Body& body, const Vector3f& force) {
	body.force += force * body.mass;
}

inline void Physics::applyForce(Physics::Body& body, const Vector3f& force,
		const Vector3f& worldPoint) {
	body.force += force * body.mass;
	body.torque += Math::cross(worldPoint - body.worldCenter, force);
}

inline void Physics::applyTorque(Physics::Body& body,
		const Vector3f& torque) {
	body.torque += torque;
}

inline void Physics::applyImpulse(Physics::Body& body,
		const Vector3f& impulse) {
	body.velocity += impulse;
}

inline void Physics::applyImpulse(Physics::Body& body, const Vector3f& impulse,
		const Vector3f& worldPoint) {
	body.velocity += impulse;
	body.angularVelocity += body.invInertiaWorld
		* Math::cross(worldPoint - body.worldCenter, impulse);
}

