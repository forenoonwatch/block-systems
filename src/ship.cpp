#include "ship.hpp"

#include "util-components.hpp"

#include "game-render-context.hpp"

#include <engine/game/game.hpp>
#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

void shipRenderSystem(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, Ship>().each([&](TransformComponent& transform,
			Ship& ship) {
		for (ArrayList<Block>::const_iterator it = ship.blocks.cbegin(),
				end = ship.blocks.cend(); it != end; ++it) {
			((GameRenderContext*)game.getRenderContext())->renderMesh(
					*((VertexArray*)BlockInfo::getInfo(it->type).vertexArray),
					*((Material*)BlockInfo::getInfo(it->type).material),
					transform.transform * it->offset);
		}
	});
}

