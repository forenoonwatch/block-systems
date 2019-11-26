#include "camera.hpp"

#include "util-components.hpp"

#include <engine/core/application.hpp>
#include <engine/game/game.hpp>

void UpdateCameraSystem::operator()(Game& game, float deltaTime) {
	game.getECS().view<TransformComponent, CameraComponent>().each([&](
			TransformComponent& transform, CameraComponent& cc) {
		Camera& camera = *cc.camera;

		camera.iView = Math::inverse(camera.view);

		const Matrix4f iProjection = Math::inverse(camera.projection);
		
		camera.viewProjection = camera.projection * camera.iView;
		camera.iViewProjection = camera.view * iProjection;

		// mouse ray
		const float ndcX = (2.f * Application::getMouseX())
				/ (float)game.getWindow().getWidth() - 1.f;
		const float ndcY = (2.f * Application::getMouseY())
				/ (float)game.getWindow().getHeight() - 1.f;

		Vector4f rawRay = iProjection
				* Vector4f(ndcX, -ndcY, -1.f, 1.f);
		rawRay = camera.view * Vector4f(rawRay.x, rawRay.y, -1.f, 0.f);
	
		cc.rayDirection = Math::normalize(Vector3f(rawRay));
	});
}

