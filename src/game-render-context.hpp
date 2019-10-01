#pragma once

#include <engine/core/tree-map.hpp>
#include <engine/core/array-list.hpp>

#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/shader.hpp>
#include <engine/rendering/render-target.hpp>
#include <engine/rendering/material.hpp>

#include <engine/game/game.hpp>

#include "camera.hpp"

class GaussianBlur;

class GameRenderContext : public RenderContext {
	public:
		GameRenderContext(uint32 width, uint32 height,
				const Matrix4f& projection);

		inline void updateCameraBuffer();

		inline void renderMesh(VertexArray& vertexArray, Material& material,
				const Matrix4f& transform);
		inline void renderSkybox(CubeMap& skybox, Sampler& sampler);

		inline virtual void setGame(Game& game) override {
			this->game = &game;
		}

		inline void setDiffuseIBL(CubeMap& diffuseIBL) { this->diffuseIBL = &diffuseIBL; }
		inline void setSpecularIBL(CubeMap& specularIBL) { this->specularIBL = &specularIBL; }
		inline void setBrdfLUT(Texture& brdfLUT) { this->brdfLUT = &brdfLUT; }

		inline CubeMap& getDiffuseIBL() { return *diffuseIBL; }
		inline CubeMap& getSpecularIBL() { return *specularIBL; }

		inline Sampler& getNearestSampler() { return nearestSampler; }
		inline Sampler& getLinearSampler() { return linearSampler; }
		inline Sampler& getLinearMipmapSampler() { return linearMipmapSampler; }

		inline VertexArray& getSkyboxCube() { return *skyboxCube; }

		inline Camera& getCamera() { return camera; }

		virtual ~GameRenderContext();
		
		/* Pipeline functions */
		static void clear(Game& game, float deltaTime);
		static void applyLighting(Game& game, float deltaTime);
		static void flushStaticMeshes(Game& game, float deltaTime);
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

		Texture colorBuffer;
		Texture normalBuffer;
		Texture lightingBuffer;
		Texture brightBuffer;
		Texture depthBuffer;

		RenderTarget target;
		RenderTarget screen;

		Shader staticMeshShader;
		Shader skyboxShader;
		Shader lightingShader;
		Shader blurShader;
		Shader bloomShader;
		Shader toneMapShader;
		Shader screenRenderShader;

		UniformBuffer sceneDataBuffer;
		UniformBuffer lightDataBuffer;

		Sampler nearestSampler;
		Sampler linearSampler;
		Sampler linearMipmapSampler;

		VertexArray* skyboxCube;

		GaussianBlur* bloomBlur;

		CubeMap* diffuseIBL;
		CubeMap* specularIBL;

		Texture* brdfLUT;

		Camera camera;

		TreeMap<Pair<VertexArray*, Material*>, ArrayList<StaticMeshData>> staticMeshes;
};

inline void GameRenderContext::updateCameraBuffer() {
	sceneDataBuffer.update(&camera.view[3], sizeof(Vector3f));
	sceneDataBuffer.update(&camera.viewProjection, sizeof(Vector3f)
			+ sizeof(Vector2f) + 3 * sizeof(float), sizeof(Matrix4f));
	sceneDataBuffer.update(&camera.iViewProjection, sizeof(Vector3f)
			+ sizeof(Vector2f) + 3 * sizeof(float)
			+ sizeof(Matrix4f), sizeof(Matrix4f));
}

inline void GameRenderContext::renderMesh(VertexArray& vertexArray, Material& material,
		const Matrix4f& transform) {
	staticMeshes[std::make_pair(&vertexArray, &material)].emplace_back(camera.viewProjection
			* transform, transform);
}

inline void GameRenderContext::renderSkybox(CubeMap& skybox, Sampler& sampler) {
	skyboxShader.setSampler("skybox", skybox, sampler, 0);
	draw(target, skyboxShader, *skyboxCube, GL_TRIANGLES);
}

