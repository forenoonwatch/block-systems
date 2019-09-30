#pragma once

#include <engine/core/tree-map.hpp>
#include <engine/core/array-list.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/shader.hpp>
#include <engine/rendering/render-target.hpp>

#include <engine/game/game.hpp>

#include "camera.hpp"

class GameRenderContext : public RenderContext {
	public:
		GameRenderContext(uint32 width, uint32 height,
				const Matrix4f& projection);

		void renderMesh(VertexArray& vertexArray, Texture& texture,
				const Matrix4f& transform);

		inline virtual void setGame(Game& game) override {
			this->game = &game;
		}

		inline Camera& getCamera() { return camera; }

		inline auto& getStaticMeshes() { return staticMeshes; }

		static void flush(Game& game, float deltaTime);
	private:
		NULL_COPY_AND_ASSIGN(GameRenderContext);

		struct StaticMeshData {
			inline StaticMeshData(const Matrix4f& mvp,
						const Matrix4f& transform)
					: projectedTransform(mvp)
					, transform(transform) {}

			Matrix4f projectedTransform;
			Matrix4f transform;
		};

		Game* game;

		Camera camera;

		RenderTarget screen;

		Shader staticMeshShader;

		Sampler linearSampler;

		TreeMap<Pair<VertexArray*, Texture*>, ArrayList<StaticMeshData>> staticMeshes;

		void flushStaticMeshes();
};
