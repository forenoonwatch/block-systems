#pragma once

#include "block.hpp"

class Game;
class VertexArray;

struct Ship {
	ArrayList<Block> blocks;
};

void shipRenderSystem(Game& game, float deltaTime);

