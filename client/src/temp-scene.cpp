#include "temp-scene.hpp"

#include <engine/core/hashed-string.hpp>

#include <engine/ecs/ecs.hpp>
#include <engine/application/application.hpp>
#include <engine/rendering/render-system.hpp>
#include <engine/scene/scene-manager.hpp>

#include <engine/resource/asset-loader.hpp>
#include <engine/resource/texture-loader.hpp>
#include <engine/resource/cube-map-loader.hpp>
#include <engine/resource/material-loader.hpp>
#include <engine/resource/shader-loader.hpp>
#include <engine/resource/model-loader.hpp>
#include <engine/resource/vertex-array-loader.hpp>
#include <engine/resource/animation-loader.hpp>
#include <engine/resource/rig-loader.hpp>
#include <engine/resource/font-loader.hpp>

#include <engine/resource/resource-manager.hpp>

#include <engine/components/player-input.hpp>
#include <engine/components/camera-component.hpp>
#include <engine/components/transform-component.hpp>
#include <engine/components/static-mesh.hpp>
#include <engine/components/rigged-mesh.hpp>
#include <engine/components/animator.hpp>

#include "orbit-camera.hpp"
#include "player-controller.hpp"

#include <entt/entity/helper.hpp> // entt::tag

void TempScene::load() {
	Application::ref().moveToCenter();

	ArrayList<IndexedModel> models;
	ArrayList<Rig> rigs;
	ArrayList<Animation> animations;

	AssetLoader::loadAssets("./res/models/sphere.obj", models, rigs, animations);
	AssetLoader::loadAssets("./res/models/plane.obj", models, rigs, animations);
	AssetLoader::loadAssets("./res/models/cube.obj", models, rigs, animations);
	AssetLoader::loadAssets("./res/models/capsule.obj", models, rigs, animations);
	AssetLoader::loadAssets("./res/models/hms-leopard-tex.fbx", models, rigs, animations);

	AssetLoader::loadAssets("./res/models/model.dae", models, rigs, animations);

	auto& rm = ResourceManager::ref();

	rm.vertexArrays.load<VertexArrayLoader>("sphere"_hs, models[0]);
	rm.vertexArrays.load<VertexArrayLoader>("plane"_hs, models[1]);
	rm.vertexArrays.load<VertexArrayLoader>("cube"_hs, models[2]);
	rm.vertexArrays.load<VertexArrayLoader>("capsule"_hs, models[3]);

	rm.vertexArrays.load<VertexArrayLoader>("ship"_hs, models[4]);

	rm.vertexArrays.load<VertexArrayLoader>("cowboy"_hs, models[5]);

	rm.models.load<ModelLoader>("cube"_hs, models[2]);

	rm.rigs.load<RigLoader>("cowboy"_hs, rigs[0]);
	rm.animations.load<AnimationLoader>("cowboy-run"_hs, animations[0]);

	auto bricksDiffuse = rm.textures.load<TextureLoader>("bricks-diffuse"_hs, "./res/textures/bricks.dds");
	auto bricksNormal = rm.textures.load<TextureLoader>("bricks-normal"_hs, "./res/textures/bricks-normal.dds");
	//auto bricksMaterial = rm.textures
	//		.load<TextureLoader>("bricks-material"_hs,
	//		"./res/textures/bricks-material.dds");
	auto bricksMaterial = rm.textures.load<TextureLoader>("bricks-material"_hs, "./res/textures/flat-material.png");

	auto bricks2Diff = rm.textures.load<TextureLoader>("bricks2-diffuse"_hs, "./res/textures/bricks2.jpg");
	auto bricks2Norm = rm.textures.load<TextureLoader>("bricks2-normal"_hs, "./res/textures/bricks2_normal.jpg");
	auto bricks2Disp = rm.textures.load<TextureLoader>("bricks2-disp"_hs, "./res/textures/bricks2_disp.jpg");

	auto flatNormal = rm.textures.load<TextureLoader>("flat-normal"_hs, "./res/textures/flat-normal.png");

	auto shipDiffuse = rm.textures
			.load<TextureLoader>("ship-diffuse"_hs, "./res/textures/Grayscale_Target.png");
	auto shipNormal = rm.textures
			.load<TextureLoader>("ship-normal"_hs, "./res/textures/Normal_Target_2.png");
	auto shipMaterial = rm.textures
			.load<TextureLoader>("ship-material"_hs, "./res/textures/Ship_Material.png");
	auto shipDisplacement = rm.textures
			.load<TextureLoader>("ship-disp"_hs, "./res/textures/ship-displacement.png");

	rm.materials.load<MaterialLoader>("bricks"_hs,
			bricksDiffuse, bricksNormal, bricksMaterial, bricks2Disp, 0.01f);
	rm.materials.load<MaterialLoader>("ship"_hs,
			shipDiffuse, shipNormal, shipMaterial, shipDisplacement, 0.001f);
	rm.materials.load<MaterialLoader>("bricks2"_hs, bricks2Diff,
			bricks2Norm, bricksMaterial, bricks2Disp, 0.01f);

	rm.shaders.load<ShaderLoader>("normal-shader"_hs, "./res/shaders/normal-shader.glsl");

	rm.fonts.load<FontLoader>("font"_hs, "/usr/share/fonts/truetype/hack/Hack-Regular.ttf", 24);

	String cubeMap = "./res/textures/sargasso-diffuse.dds";
	rm.cubeMaps.load<CubeMapLoader>("sargasso-diffuse"_hs, &cubeMap, 1);

	cubeMap = "./res/textures/sargasso-specular.dds";
	rm.cubeMaps.load<CubeMapLoader>("sargasso-specular"_hs, &cubeMap, 1);

	rm.textures.load<TextureLoader>("schlick-brdf"_hs, "./res/textures/schlick-brdf.png");

	RenderSystem::ref().setDiffuseIBL(rm.cubeMaps.handle("sargasso-diffuse"_hs));
	RenderSystem::ref().setSpecularIBL(rm.cubeMaps.handle("sargasso-specular"_hs));
	RenderSystem::ref().setBrdfLUT(rm.textures.handle("schlick-brdf"_hs));
	
	auto& registry = Registry::ref();

	auto rot = Math::rotate(Quaternion(1, 0, 0, 0), Math::toRadians(-90.f), Vector3f(1, 0, 0));
	//auto rot = Quaternion(1, 0, 0, 0);

	auto plane = registry.create();
	registry.assign<TransformComponent>(plane, Transform());
	registry.assign<StaticMesh>(plane, &rm.vertexArrays.handle("plane"_hs).get(),
			&rm.materials.handle("bricks2"_hs).get(), true);

	auto test = registry.create();
	registry.assign<TransformComponent>(test, Transform(Vector3f(0, 0, 5)));
	registry.assign<StaticMesh>(test, &rm.vertexArrays.handle("cube"_hs).get(),
			&rm.materials.handle("bricks2"_hs).get(), true);
	registry.assign<Tag<"spinner"_hs>>(test);

	auto plr = registry.create();
	registry.assign<StaticMesh>(plr, &rm.vertexArrays.handle("capsule"_hs).get(),
			&rm.materials.handle("bricks2"_hs).get(), true);
	registry.assign<TransformComponent>(plr, Transform(Vector3f(0, 1, 0)));
	registry.assign<CameraComponent>(plr, Vector3f(), 0, 0);
	registry.assign<CameraDistanceComponent>(plr, 0.1f, 0.1f, 50.f);
	registry.assign<PlayerInputComponent>(plr);
	registry.assign<PlayerController>(plr, Vector3f(), 5.f);
}

void TempScene::update(float deltaTime) {
	auto& registry = Registry::ref();
	auto& app = Application::ref();
	auto& renderer = RenderSystem::ref();

	app.pollEvents();

	updatePlayerController(registry, deltaTime);

	if (app.getKeyPressed(Input::KEY_M)) {
		app.setFullscreen(!app.isFullscreen());
		//auto& monitor = app.getPrimaryMonitor();
		//app.resizeWindow(monitor.getWidth(), monitor.getHeight());
	}

	updatePlayerInput(registry, app, deltaTime);

	orbitCameraSystem(registry, app, renderer, deltaTime);
	updateCameraComponents(registry, app, renderer);

	updateAnimators(registry, deltaTime);

	Vector3f lookTarget;

	registry.view<TransformComponent, PlayerController>().each([&](auto& tfc, auto& pc) {
		lookTarget = tfc.transform.getPosition();
	});

	registry.view<Tag<"spinner"_hs>>().each([&](auto& entity, auto tag) {
		auto& tfc = registry.get<TransformComponent>(entity);
		tfc.transform.matrixLookAt(lookTarget);
	});

	renderer.updateCamera();
}

void TempScene::render() {
	auto& registry = Registry::ref();
	auto& renderer = RenderSystem::ref();
	auto& context = RenderContext::ref();

	auto& rm = ResourceManager::ref();

	auto font = rm.fonts.handle("font"_hs);

	String fpsText = "FPS:" + std::to_string(SceneManager::ref().getFPS());

	renderer.drawText(font, fpsText, 20, 560, Vector3f(0, 0, 0));

	renderStaticMeshes(registry, renderer);
	renderRiggedMeshes(registry, renderer);

	renderer.clear();

	renderer.flushStaticMeshes();
	renderer.flushRiggedMeshes();
	renderer.applyLighting();
	
	renderer.renderSkybox();

	renderer.flush();

	renderer.flushTexturedQuads();

	Application::ref().swapBuffers();
}

void TempScene::unload() {
}
