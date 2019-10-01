#include "game-render-context.hpp"

GameRenderContext::GameRenderContext(uint32 width, uint32 height,
			const Matrix4f& projection)
		: game(nullptr)
		, camera({projection, Matrix4f(1.f), projection,
				Matrix4f(1.f), Math::inverse(projection)})
		, screen(*((RenderContext*)this), width, height)
		, staticMeshShader(*((RenderContext*)this))
		, linearSampler(*((RenderContext*)this), GL_LINEAR, GL_LINEAR)
		, linearMipmapSampler(*((RenderContext*)this), GL_LINEAR_MIPMAP_LINEAR,
				GL_LINEAR_MIPMAP_LINEAR) {
	staticMeshShader.load("./res/shaders/static-mesh-deferred.glsl");
}

void GameRenderContext::flush(Game& game, float deltaTime) {
	((GameRenderContext*)game.getRenderContext())->screen.clear(
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	((GameRenderContext*)game.getRenderContext())->flushStaticMeshes();
}

inline void GameRenderContext::flushStaticMeshes() {
	Material* currentMaterial = nullptr;
	Material* material;

	VertexArray* vertexArray;
	uintptr numTransforms;

	for (auto it = std::begin(staticMeshes), end = std::end(staticMeshes);
			it != end; ++it) {
		numTransforms = it->second.size();

		if (numTransforms == 0) {
			continue;
		}

		vertexArray = it->first.first;
		material = it->first.second;

		if (material != currentMaterial) {
			currentMaterial = material;

			staticMeshShader.setSampler("diffuse", *material->diffuse,
					linearMipmapSampler, 0);
			staticMeshShader.setSampler("normalMap", *material->normalMap,
					linearMipmapSampler, 1);
			staticMeshShader.setSampler("materialMap", *material->materialMap,
					linearMipmapSampler, 2);
		}

		vertexArray->updateBuffer(4, &it->second[0],
				sizeof(StaticMeshData) * numTransforms);

		draw(screen, staticMeshShader, *vertexArray, GL_TRIANGLES, numTransforms);
	}

	staticMeshes.clear();
}
