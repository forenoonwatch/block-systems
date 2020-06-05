#include "temp-scene.hpp"

#include <engine/core/hashed-string.hpp>

#include <engine/ecs/ecs.hpp>
#include <engine/application/application.hpp>
#include <engine/rendering/render-system.hpp>

#include <engine/resource/texture-loader.hpp>
#include <engine/resource/cube-map-loader.hpp>
#include <engine/resource/material-loader.hpp>
#include <engine/resource/shader-loader.hpp>

#include <engine/resource/model-loader.hpp>
#include <engine/resource/static-mesh-loader.hpp>

#include <engine/resource/resource-cache.hpp>

#include <engine/components/player-input.hpp>
#include <engine/components/camera-component.hpp>
#include <engine/components/transform-component.hpp>
#include <engine/components/static-mesh.hpp>

#include "orbit-camera.hpp"

void TempScene::load() {
	Application::ref().moveToCenter();

	ResourceCache<VertexArray>::ref().load<StaticMeshLoader>("sphere"_hs, "./res/models/sphere.obj");
	ResourceCache<VertexArray>::ref().load<StaticMeshLoader>("plane"_hs, "./res/models/plane.obj");
	ResourceCache<VertexArray>::ref().load<StaticMeshLoader>("cube"_hs, "./res/models/cube.obj");
	ResourceCache<VertexArray>::ref().load<StaticMeshLoader>("capsule"_hs, "./res/models/capsule.obj");

	ResourceCache<VertexArray>::ref().load<StaticMeshLoader>("ship"_hs, "./res/models/hms-leopard-tex.fbx");

	ResourceCache<IndexedModel>::ref().load<ModelLoader>("cube"_hs, "./res/models/cube.obj");

	auto bricksDiffuse = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-diffuse"_hs, "./res/textures/bricks.dds");
	auto bricksNormal = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-normal"_hs,
			"./res/textures/bricks-normal.dds");
	//auto bricksMaterial = ResourceCache<Texture>::ref()
	//		.load<TextureLoader>("bricks-material"_hs,
	//		"./res/textures/bricks-material.dds");
	auto bricksMaterial = ResourceCache<Texture>::ref()
			.load<TextureLoader>("bricks-material"_hs,
			"./res/textures/flat-material.png");

	auto bricks2Diff = ResourceCache<Texture>::ref().load<TextureLoader>("bricks2-diffuse"_hs, "./res/textures/bricks2.jpg");
	auto bricks2Norm = ResourceCache<Texture>::ref().load<TextureLoader>("bricks2-normal"_hs, "./res/textures/bricks2_normal.jpg");
	auto bricks2Disp = ResourceCache<Texture>::ref().load<TextureLoader>("bricks2-disp"_hs, "./res/textures/bricks2_disp.jpg");

	auto flatNormal = ResourceCache<Texture>::ref().load<TextureLoader>("flat-normal"_hs, "./res/textures/flat-normal.png");

	auto shipDiffuse = ResourceCache<Texture>::ref()
			.load<TextureLoader>("ship-diffuse"_hs, "./res/textures/Grayscale_Target.png");
	auto shipNormal = ResourceCache<Texture>::ref()
			.load<TextureLoader>("ship-normal"_hs, "./res/textures/Normal_Target_2.png");
	auto shipMaterial = ResourceCache<Texture>::ref()
			.load<TextureLoader>("ship-material"_hs, "./res/textures/Ship_Material.png");
	auto shipDisplacement = ResourceCache<Texture>::ref()
			.load<TextureLoader>("ship-disp"_hs, "./res/textures/ship-displacement.png");

	ResourceCache<Material>::ref().load<MaterialLoader>("bricks"_hs,
			bricksDiffuse, bricksNormal, bricksMaterial, bricks2Disp);
	ResourceCache<Material>::ref().load<MaterialLoader>("ship"_hs,
			shipDiffuse, shipNormal, shipMaterial, shipDisplacement);
	ResourceCache<Material>::ref().load<MaterialLoader>("bricks2"_hs, bricks2Diff, bricks2Norm, bricksMaterial, bricks2Disp);

	ResourceCache<Shader>::ref().load<ShaderLoader>("normal-shader"_hs, "./res/shaders/normal-shader.glsl");

	String cubeMap = "./res/textures/sargasso-diffuse.dds";
	ResourceCache<CubeMap>::ref()
			.load<CubeMapLoader>("sargasso-diffuse"_hs, &cubeMap, 1);
	cubeMap = "./res/textures/sargasso-specular.dds";
	ResourceCache<CubeMap>::ref()
			.load<CubeMapLoader>("sargasso-specular"_hs, &cubeMap, 1);

	ResourceCache<Texture>::ref()
			.load<TextureLoader>("schlick-brdf"_hs, "./res/textures/schlick-brdf.png");

	RenderSystem::ref().setDiffuseIBL(ResourceCache<CubeMap>::ref().handle("sargasso-diffuse"_hs));
	RenderSystem::ref().setSpecularIBL(ResourceCache<CubeMap>::ref().handle("sargasso-specular"_hs));
	RenderSystem::ref().setBrdfLUT(ResourceCache<Texture>::ref().handle("schlick-brdf"_hs));
	
	auto& registry = Registry::ref();

	auto rot = Math::rotate(Quaternion(1, 0, 0, 0), Math::toRadians(-90.f), Vector3f(1, 0, 0));
	//auto rot = Quaternion(1, 0, 0, 0);

	auto cube = registry.create();
	registry.assign<TransformComponent>(cube, Transform(Vector3f(0, 0, 0), rot, Vector3f(1, 1, 1)));
	registry.assign<StaticMesh>(cube, &ResourceCache<VertexArray>::ref().handle("ship"_hs).get(),
			&ResourceCache<Material>::ref().handle("ship"_hs).get(), true);

	auto eCam = registry.create();

	registry.assign<TransformComponent>(eCam, Transform(Vector3f(0, 0, 0)));
	registry.assign<CameraComponent>(eCam, Vector3f(), 0, 0);
	registry.assign<CameraDistanceComponent>(eCam, 0.1f, 0.1f, 50.f);
	registry.assign<PlayerInputComponent>(eCam);
}

void TempScene::update(float deltaTime) {
	auto& registry = Registry::ref();
	auto& app = Application::ref();
	auto& renderer = RenderSystem::ref();

	app.pollEvents();

	if (app.getKeyPressed(Input::KEY_M)) {
		//app.setFullscreen(!app.isFullscreen());
		auto& monitor = app.getPrimaryMonitor();
		app.resizeWindow(monitor.getWidth(), monitor.getHeight());
	}

	updatePlayerInput(registry, app, deltaTime);

	orbitCameraSystem(registry, app, renderer, deltaTime);
	updateCameraComponents(registry, app, renderer);

	registry.view<TransformComponent, StaticMesh>().each([deltaTime](auto& tfc, auto& sm) {
		//tfc.transform.setRotation(Math::rotate(tfc.transform.getRotation(), deltaTime, Vector3f(0, 1, 0)));
	});

	renderer.updateCamera();
}

void TempScene::render() {
	auto& registry = Registry::ref();
	auto& renderer = RenderSystem::ref();
	auto& context = RenderContext::ref();

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
