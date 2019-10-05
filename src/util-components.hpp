#pragma once

#include <engine/math/matrix.hpp>

struct TransformComponent {
	Matrix4f transform;
};

struct MotionComponent {
	Vector3f velocity;
	Vector3f acceleration;
};

