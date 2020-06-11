#pragma once

#include <engine/math/vector.hpp>

class Registry;

struct PlayerController {
    Vector3f moveDirection;
    float moveSpeed;
};

void updatePlayerController(Registry& registry, float deltaTime);
