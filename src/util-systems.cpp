#include "util-systems.hpp"

#include "camera.hpp"
#include "renderable-mesh.hpp"

#include "util-components.hpp"

#include "game-render-context.hpp"

#include <engine/ecs/ecs.hpp>

#include <engine/core/application.hpp>

void RenderMesh::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, RenderableMesh>().each([&](
			TransformComponent& transform, RenderableMesh& mesh) {
		if (mesh.render) {
			((GameRenderContext*)game.getRenderContext())->renderMesh(
					*mesh.vertexArray, *mesh.material,
					transform.transform.toMatrix());
		}
	});
}

void RenderSkybox::operator()(Game& game, float deltaTime) {
	GameRenderContext* grc = (GameRenderContext*)game.getRenderContext();

	Matrix4f mvp = Math::translate(grc->getCamera().viewProjection,
			Vector3f(grc->getCamera().view[3]));

	grc->getSkyboxCube().updateBuffer(1, &mvp, sizeof(Matrix4f));
	grc->renderSkybox(grc->getSpecularIBL(), grc->getLinearMipmapSampler());
}

void ToggleFullscreenSystem::operator()(Game& game, float deltaTime) {
	if (Application::getKeyPressed(Input::KEY_M)) {
		if (game.getWindow().isFullscreen()) {
			game.getWindow().setFullscreen(false);
			game.getWindow().resize(1200, 800);
			game.getWindow().moveToCenter();
		}
		else {
			game.getWindow().setFullscreen(true);
		}
	}
	else if (Application::getKeyPressed(Input::KEY_ESCAPE)
			&& game.getWindow().isFullscreen()) {
		game.getWindow().setFullscreen(false);
		game.getWindow().resize(1200, 800);
		game.getWindow().moveToCenter();
	}
}

