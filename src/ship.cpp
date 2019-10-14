#include "ship.hpp"

#include "util-components.hpp"
#include "renderable-mesh.hpp"

#include "game-render-context.hpp"

#include <engine/game/game.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

void shipRenderSystem(Game& game, float deltaTime) {
	Shader& shader = game.getAssetManager().getShader("ship-shader");
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();
	Material* currentMaterial = nullptr;

	game.getECS().view<TransformComponent, Ship>().each([&](
			TransformComponent& transform, Ship& ship) {
		uint32 numTransforms;

		shader.setMatrix4f("baseTransform", transform.transform);

		for (auto& pair : ship.offsets) {
			numTransforms = pair.second.size();

			if (numTransforms == 0) {
				continue;
			}

			Material* material = ship.blockInfo[pair.first].material;

			if (material != currentMaterial) {
				currentMaterial = material;

				shader.setSampler("diffuse", *material->diffuse,
						grc->getLinearMipmapSampler(), 0);
				shader.setSampler("normalMap", *material->normalMap,
						grc->getLinearMipmapSampler(), 1);
				shader.setSampler("materialMap", *material->materialMap,
						grc->getLinearMipmapSampler(), 2);
			}

			grc->draw(grc->getTarget(), shader,
					*ship.blockInfo[pair.first].vertexArray, GL_TRIANGLES,
					numTransforms);
		}
	});
}

