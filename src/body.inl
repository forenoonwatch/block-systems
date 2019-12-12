
inline void Physics::Body::applyForce(const Vector3f& force) {
	this->force += force * mass;

	setToAwake();
}

inline void Physics::Body::applyForce(const Vector3f& force,
		const Vector3f& worldPoint) {
	this->force += force * mass;
	this->torque += Math::cross(worldPoint - worldCenter, force);

	setToAwake();
}

inline void Physics::Body::applyTorque(const Vector3f& torque) {
	this->torque += torque;
}

inline void Physics::Body::applyImpulse(const Vector3f& impulse) {
	velocity += impulse * invMass;

	setToAwake();
}

inline void Physics::Body::applyImpulse(const Vector3f& impulse,
		const Vector3f& worldPoint) {
	velocity += impulse * invMass;
	angularVelocity += invInertiaWorld
		* Math::cross(worldPoint - worldCenter, impulse);
	
	setToAwake();
}

inline Vector3f Physics::Body::getVelocityAt(const Vector3f& worldPoint)
		const {
	return velocity + Math::cross(angularVelocity, worldPoint - worldCenter);
}

inline bool Physics::Body::canCollideWith(const Body& other) const {
	if (this == &other) {
		return false;
	}

	if (!isDynamic() && !other.isDynamic()) {
		return false;
	}

	if (!(collisionGroups & other.collisionGroups)) {
		return false;
	}

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
	if (!(flags & Physics::Body::FLAG_AWAKE)) {
		flags |= Physics::Body::FLAG_AWAKE;
		sleepTime = 0.f;
	}
}

inline void Physics::Body::setToSleep() {
	flags &= ~Physics::Body::FLAG_AWAKE;
	sleepTime = 0.f;

	velocity = Vector3f();
	angularVelocity = Vector3f();
	force = Vector3f();
	torque = Vector3f();
}

inline void Physics::Body::setInIsland() {
	flags |= Physics::Body::FLAG_ISLAND;
}

inline void Physics::Body::setNotInIsland() {
	flags &= ~Physics::Body::FLAG_ISLAND;
}

inline void Physics::Body::setTransform(const Transform& transform) {
	this->worldCenter = transform.getPosition();
	this->transform.setRotation(transform.getRotation());

	updateBroadphase();
}

inline void Physics::Body::setVelocity(const Vector3f& velocity) {
	this->velocity = velocity;
}

inline void Physics::Body::setAngularVelocity(const Vector3f& angularVelocity) {
	this->angularVelocity = angularVelocity;
}

inline void Physics::Body::setForce(const Vector3f& force) {
	this->force = force;
}

inline void Physics::Body::setTorque(const Vector3f& torque) {
	this->torque = torque;
}

inline void Physics::Body::setMass(float mass) {
	this->mass = mass;
	invMass = mass == 0.f ? 0.f : 1.f / mass;
}

inline void Physics::Body::setGravityScale(float gravityScale) {
	this->gravityScale = gravityScale;
}

inline void Physics::Body::setCollisionGroup(uint32 groupID, bool canCollide) {
	collisionGroups |= ((uint32)canCollide << groupID);
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

inline const Transform& Physics::Body::getTransform() const {
	return transform;
}

inline Vector3f& Physics::Body::getLocalCenter() {
	return localCenter;
}

inline const Vector3f& Physics::Body::getLocalCenter() const {
	return localCenter;
}

inline const Vector3f& Physics::Body::getWorldCenter() const {
	return worldCenter;
}

inline Vector3f& Physics::Body::getVelocity() {
	return velocity;
}

inline const Vector3f& Physics::Body::getVelocity() const {
	return velocity;
}

inline Vector3f& Physics::Body::getAngularVelocity() {
	return angularVelocity;
}

inline const Vector3f& Physics::Body::getAngularVelocity() const {
	return angularVelocity;
}

inline Vector3f& Physics::Body::getForce() {
	return force;
}

inline const Vector3f& Physics::Body::getForce() const {
	return force;
}

inline Vector3f& Physics::Body::getTorque() {
	return torque;
}

inline const Vector3f& Physics::Body::getTorque() const {
	return torque;
}

inline float Physics::Body::getMass() const {
	return mass;
}

inline float Physics::Body::getInvMass() const {
	return invMass;
}

inline Matrix3f& Physics::Body::getInvInertiaLocal() {
	return invInertiaLocal;
}

inline const Matrix3f& Physics::Body::getInvInertiaLocal() const {
	return invInertiaLocal;
}

inline const Matrix3f& Physics::Body::getInvInertiaWorld() const {
	return invInertiaWorld;
}

inline float Physics::Body::getGravityScale() const {
	return gravityScale;
}

inline bool Physics::Body::isInCollisionGroup(uint32 groupID) const {
	return collisionGroups & (1 << groupID);
}

inline uint32 Physics::Body::getCollisionGroups() const {
	return collisionGroups;
}

inline const ArrayList<Physics::Collider*>& Physics::Body::getColliders()
		const {
	return colliders;
}

