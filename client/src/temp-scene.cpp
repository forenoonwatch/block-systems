#include "temp-scene.hpp"

#include <engine/core/hashed-string.hpp>

#include <engine/ecs/ecs.hpp>
#include <engine/application/application.hpp>
#include <engine/rendering/render-system.hpp>

#include <engine/resource/texture-loader.hpp>
#include <engine/resource/cube-map-loader.hpp>
#include <engine/resource/material-loader.hpp>

#include <engine/resource/model-loader.hpp>
#include <engine/resource/vertex-array-loader.hpp>

#include <engine/resource/resource-cache.hpp>

#include <engine/components/player-input.hpp>
#include <engine/components/camera-component.hpp>
#include <engine/components/transform-component.hpp>
#include <engine/components/static-mesh.hpp>

#include "orbit-camera.hpp"

void TempScene::load() {
	Application::ref().moveToCenter();

	struct IndexedModel::AllocationHints hints;
		hints.elementSizes.push_back(3);
		hints.elementSizes.push_back(2);
		hints.elementSizes.push_back(3);
		hints.elementSizes.push_back(3);
		hints.elementSizes.push_back(16);
		hints.instancedElementStartIndex = 4;

	ResourceCache<VertexArray>::ref()
			.load<VertexArrayLoader>("sphere"_hs, "./res/sphere.obj", hints);
	ResourceCache<VertexArray>::ref()
			.load<VertexArrayLoader>("plane"_hs, "./res/plane.obj", hints);
	ResourceCache<VertexArray>::ref()
			.load<VertexArrayLoader>("cube"_hs, "./res/cube.obj", hints);
	ResourceCache<VertexArray>::ref()
			.load<VertexArrayLoader>("capsule"_hs, "./res/capsule.obj", hints);
	
	ResourceCache<VertexArray>::ref()
			.load<VertexArrayLoader>("platform"_hs, "./res/platform.obj", hints);

	ResourceCache<IndexedModel>::ref()
			.load<ModelLoader>("cube"_hs, "./res/cube.obj", hints);

	auto bricksDiffuse = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-diffuse"_hs, "./res/bricks.dds");
	auto bricksNormal = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-normal"_hs,
			"./res/bricks-normal.dds");
	auto bricksMaterial = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-material"_hs,
			"./res/bricks-material.dds");

	ResourceCache<Material>::ref().load<MaterialLoader>("bricks"_hs,
			bricksDiffuse, bricksNormal, bricksMaterial);

	String cubeMap = "./res/sargasso-diffuse.dds";
	ResourceCache<CubeMap>::ref()
			.load<CubeMapLoader>("sargasso-diffuse"_hs, &cubeMap, 1);
	cubeMap = "./res/sargasso-specular.dds";
	ResourceCache<CubeMap>::ref()
			.load<CubeMapLoader>("sargasso-specular"_hs, &cubeMap, 1);

	ResourceCache<Texture>::ref()
			.load<TextureLoader>("schlick-brdf"_hs, "./res/schlick-brdf.png");

	RenderSystem::ref().setDiffuseIBL(ResourceCache<CubeMap>::ref().handle("sargasso-diffuse"_hs));
	RenderSystem::ref().setSpecularIBL(ResourceCache<CubeMap>::ref().handle("sargasso-specular"_hs));
	RenderSystem::ref().setBrdfLUT(ResourceCache<Texture>::ref().handle("schlick-brdf"_hs));
	
	auto& registry = Registry::ref();

	auto cube = registry.create();
	registry.assign<TransformComponent>(cube, Transform(Vector3f(0, 0, -3)));
	registry.assign<StaticMesh>(cube, &ResourceCache<VertexArray>::ref().handle("cube"_hs).get(),
			&ResourceCache<Material>::ref().handle("bricks"_hs).get(), true);

	auto eCam = registry.create();

	registry.assign<TransformComponent>(eCam, Transform(Vector3f(0, 0, -3)));
	registry.assign<CameraComponent>(eCam, Vector3f(), 0, 0);
	registry.assign<CameraDistanceComponent>(eCam, 0.1f, 0.1f, 10.f);
	registry.assign<PlayerInputComponent>(eCam);
}

void TempScene::update(float deltaTime) {
	auto& registry = Registry::ref();
	auto& app = Application::ref();
	auto& renderer = RenderSystem::ref();

	app.pollEvents();

	updatePlayerInput(registry, app, deltaTime);

	orbitCameraSystem(registry, app, renderer, deltaTime);
	updateCameraComponents(registry, app, renderer);

	registry.view<TransformComponent, StaticMesh>().each([deltaTime](auto& tfc, auto& sm) {
		tfc.transform.setRotation(Math::rotate(tfc.transform.getRotation(), deltaTime, Vector3f(0, 1, 0)));
	});

	renderer.updateCamera();
}

void TempScene::render() {
	auto& registry = Registry::ref();
	auto& renderer = RenderSystem::ref();

	renderStaticMeshes(registry, renderer);

	renderer.clear();
	renderer.flushStaticMeshes();
	renderer.applyLighting();
	
	renderer.renderSkybox();

	renderer.flush();

	Application::ref().swapBuffers();
}

void TempScene::unload() {
}

namespace {
	//Application::ref().renderMesh(
	//		ResourceCache<VertexArray>::ref()
	//		.handle("cube"_hs), material,
	//		coll->getWorldTransform().toMatrix());
};
