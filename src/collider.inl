
inline Physics::ColliderHints::ColliderHints()
		: type(Physics::ColliderType::TYPE_INVALID)
		, transform()
		, density(1.f)
		, friction(0.f)
		, restitution(0.f) {}

inline void Physics::ColliderHints::setTransform(const Transform& transform) {
	this->transform = transform;
}

inline void Physics::ColliderHints::setDensity(float density) {
	this->density = density;
}

inline void Physics::ColliderHints::setFriction(float friction) {
	this->friction = friction;
}

inline void Physics::ColliderHints::setRestitution(float restitution) {
	this->restitution = restitution;
}

inline float Physics::ColliderHints::getRadius() const {
	return radius;
}

inline const Vector3f& Physics::ColliderHints::getP0() const {
	return p0;
}

inline const Vector3f& Physics::ColliderHints::getP1() const {
	return p1;
}

inline const IndexedModel* Physics::ColliderHints::getModel() const {
	return model;
}

inline void Physics::Collider::setFriction(float friction) {
	this->friction = friction;
}

inline void Physics::Collider::setRestitution(float restitution) {
	this->restitution = restitution;
}

inline void Physics::Collider::setWorldTransform(const Transform& transform) {
	worldTransform = transform;
}

inline const Transform& Physics::Collider::getLocalTransform() const {
	return localTransform;
}

inline const Transform& Physics::Collider::getWorldTransform() const {
	return worldTransform;
}

inline float Physics::Collider::getDensity() const {
	return density;
}

inline float Physics::Collider::getFriction() const {
	return friction;
}

inline float Physics::Collider::getRestitution() const {
	return restitution;
}

inline Physics::Body* Physics::Collider::getBody() {
	return body;
}

inline const Physics::Body* Physics::Collider::getBody() const {
	return body;
}

inline enum Physics::ColliderType Physics::Collider::getType() const {
	return type;
}

