#pragma once

#include "block.hpp"

class Game;

struct Ship {
	ArrayList<Block> blocks;
};

void shipRenderSystem(Game& game, float deltaTime);

