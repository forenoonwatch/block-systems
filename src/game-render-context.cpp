#include "game-render-context.hpp"

GameRenderContext::GameRenderContext(uint32 width, uint32 height,
			const Matrix4f& projection)
		: game(nullptr)
		, camera({projection, Matrix4f(1.f), projection,
				Matrix4f(1.f), Math::inverse(projection)})
		, screen(*((RenderContext*)this), width, height)
		, staticMeshShader(*((RenderContext*)this))
		, linearSampler(*((RenderContext*)this), GL_LINEAR, GL_LINEAR) {
	staticMeshShader.load("./res/shaders/static-mesh-shader.glsl");
}

void GameRenderContext::renderMesh(VertexArray& vertexArray, Texture& texture,
		const Matrix4f& transform) {
	staticMeshes[std::make_pair(&vertexArray, &texture)].emplace_back(camera.viewProjection
			* transform, transform);
}

void GameRenderContext::flush() {
	screen.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	flushStaticMeshes();
}

inline void GameRenderContext::flushStaticMeshes() {
	Texture* currentTexture = nullptr;
	Texture* texture;

	VertexArray* vertexArray;
	uintptr numTransforms;

	for (auto it = std::begin(staticMeshes), end = std::end(staticMeshes);
			it != end; ++it) {
		numTransforms = it->second.size();

		if (numTransforms == 0) {
			continue;
		}

		vertexArray = it->first.first;
		texture = it->first.second;

		if (texture != currentTexture) {
			currentTexture = texture;
			// staticMeshShader.setSampler(...);
		}

		vertexArray->updateBuffer(4, &it->second[0],
				sizeof(StaticMeshData) * numTransforms);

		draw(screen, staticMeshShader, *vertexArray, GL_TRIANGLES, numTransforms);
	}

	staticMeshes.clear();
}
