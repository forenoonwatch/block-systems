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

#include <engine/physics/physics-engine.hpp>
#include <engine/physics/physics-util.hpp>

#include <engine/ocean/ocean.hpp>

#include "orbit-camera.hpp"
#include "player-controller.hpp"

#include <entt/entity/helper.hpp> // entt::tag

namespace {
	//Body planeBodyPtr;
};

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

	AssetLoader::loadAssets("./res/models/platform.obj", models, rigs, animations);

	auto& rm = ResourceManager::ref();

	rm.vertexArrays.load<VertexArrayLoader>("sphere"_hs, models[0]);
	rm.vertexArrays.load<VertexArrayLoader>("plane"_hs, models[1]);
	rm.vertexArrays.load<VertexArrayLoader>("cube"_hs, models[2]);
	rm.vertexArrays.load<VertexArrayLoader>("capsule"_hs, models[3]);

	rm.vertexArrays.load<VertexArrayLoader>("ship"_hs, models[4]);

	rm.vertexArrays.load<VertexArrayLoader>("cowboy"_hs, models[5]);

	rm.vertexArrays.load<VertexArrayLoader>("platform"_hs, models[6]);

	rm.models.load<ModelLoader>("cube"_hs, models[2]);

	rm.rigs.load<RigLoader>("cowboy"_hs, rigs[0]);
	rm.animations.load<AnimationLoader>("cowboy-run"_hs, animations[0]);

	auto bricksDiffuse = rm.textures.load<TextureLoader>("bricks-diffuse"_hs, "./res/textures/bricks.dds");
	auto bricksNormal = rm.textures.load<TextureLoader>("bricks-normal"_hs, "./res/textures/bricks-normal.dds");
	auto bricksMaterial = rm.textures.load<TextureLoader>("bricks-material"_hs, "./res/textures/bricks-material.dds");

	auto bricks2Diff = rm.textures.load<TextureLoader>("bricks2-diffuse"_hs, "./res/textures/bricks2.jpg");
	auto bricks2Norm = rm.textures.load<TextureLoader>("bricks2-normal"_hs, "./res/textures/bricks2_normal.jpg");
	auto bricks2Disp = rm.textures.load<TextureLoader>("bricks2-disp"_hs, "./res/textures/bricks2_disp.jpg");

	auto flatNormal = rm.textures.load<TextureLoader>("flat-normal"_hs, "./res/textures/flat-normal.png");
	auto flatMaterial = rm.textures.load<TextureLoader>("flat-material"_hs, "./res/textures/flat-material.png");
	auto flatDisp = rm.textures.load<TextureLoader>("flat-disp"_hs, "./res/textures/flat-disp.png");

	auto metalDiffuse = rm.textures.load<TextureLoader>("metal-diffuse"_hs, "./res/textures/metal-diffuse.png");
	auto metalMaterial = rm.textures.load<TextureLoader>("metal-material"_hs, "./res/textures/metal-material.png");

	auto shipDiffuse = rm.textures
			.load<TextureLoader>("ship-diffuse"_hs, "./res/textures/Grayscale_Target.png");
	auto shipNormal = rm.textures
			.load<TextureLoader>("ship-normal"_hs, "./res/textures/Normal_Target_2.png");
	auto shipMaterial = rm.textures
			.load<TextureLoader>("ship-material"_hs, "./res/textures/Ship_Material.png");
	auto shipDisplacement = rm.textures
			.load<TextureLoader>("ship-disp"_hs, "./res/textures/ship-displacement.png");

	rm.textures.load<TextureLoader>("foam"_hs, "./res/textures/foam.jpg");

	rm.materials.load<MaterialLoader>("bricks"_hs,
			bricksDiffuse, bricksNormal, bricksMaterial, bricks2Disp, 0.01f);
	rm.materials.load<MaterialLoader>("ship"_hs,
			shipDiffuse, shipNormal, shipMaterial, shipDisplacement, 0.001f);
	rm.materials.load<MaterialLoader>("bricks2"_hs, bricks2Diff,
			bricks2Norm, flatMaterial, bricks2Disp, 0.01f);
	rm.materials.load<MaterialLoader>("metal"_hs, metalDiffuse, flatNormal, metalMaterial, flatDisp, 0.f);

	rm.shaders.load<ShaderLoader>("normal-shader"_hs, "./res/shaders/normal-shader.glsl");

	//rm.fonts.load<FontLoader>("font"_hs, "/usr/share/fonts/truetype/hack/Hack-Regular.ttf", 24);
	rm.fonts.load<FontLoader>("font"_hs, "./res/fonts/LucidaTypewriterRegular.ttf", 24);

	String cubeMap = "./res/textures/sargasso-diffuse.dds";
	rm.cubeMaps.load<CubeMapLoader>("sargasso-diffuse"_hs, &cubeMap, 1);

	cubeMap = "./res/textures/sargasso-specular.dds";
	rm.cubeMaps.load<CubeMapLoader>("sargasso-specular"_hs, &cubeMap, 1);

	rm.textures.load<TextureLoader>("schlick-brdf"_hs, "./res/textures/schlick-brdf.png");

	RenderSystem::ref().setDiffuseIBL(rm.cubeMaps.handle("sargasso-diffuse"_hs));
	RenderSystem::ref().setSpecularIBL(rm.cubeMaps.handle("sargasso-specular"_hs));
	RenderSystem::ref().setBrdfLUT(rm.textures.handle("schlick-brdf"_hs));

	auto& registry = Registry::ref();
	auto& physics = PhysicsEngine::ref();

	//auto rot = Quaternion(1, 0, 0, 0);
	auto rot = Math::rotate(Quaternion(1, 0, 0, 0), Math::toRadians(45.f), Vector3f(1, 0, 0));

	auto planeCollider = physics.createCollider<BoxCollider>(Vector3f(10.f, 0.1f, 10.f));
	//auto planeBody = physics.createBody(planeCollider, 0.f, Vector3f(0, -2, 0));
	auto planeController = physics.createVehicleController(planeCollider, Vector3f(), rot);

	planeController.getController()->setLinearVelocity(btVector3(0, 0, 0.5));

	//planeBodyPtr = planeBody;

	//planeBody.applyImpulse(Vector3f(0, 0, 1));
	//planeBody.getHandle()->setAngularFactor(0.f);

	auto plane = registry.create();
	registry.assign<TransformComponent>(plane, Transform());
	registry.assign<StaticMesh>(plane, &rm.vertexArrays.handle("platform"_hs).get(),
			&rm.materials.handle("bricks2"_hs).get(), true);
	registry.assign<VehicleController>(plane, planeController);
	//registry.assign<Body>(plane, planeBody);

	auto test = registry.create();
	registry.assign<TransformComponent>(test, Transform(Vector3f(0, 2, 5)));
	registry.assign<StaticMesh>(test, &rm.vertexArrays.handle("cube"_hs).get(),
			&rm.materials.handle("bricks2"_hs).get(), true);
	registry.assign<Tag<"spinner"_hs>>(test);

	auto playerCollider = physics.createCollider<CapsuleCollider>(1.f, 1.f);
	//auto playerBody = physics.createBody(playerCollider, 1.f, Vector3f(0, 15.f, 0));
	//playerBody.setInvInertiaDiagLocal(Vector3f());

	rot = Math::rotate(Quaternion(1, 0, 0, 0), Math::toRadians(90.f), Vector3f(1, 0, 0));

	auto playerCC = physics.createCharacterController(playerCollider, 0.1, Vector3f(0, 1, 0), rot);
	playerCC.getController()->setJumpSpeed(5.f);

	auto plr = registry.create();
	registry.assign<StaticMesh>(plr, &rm.vertexArrays.handle("capsule"_hs).get(),
			&rm.materials.handle("metal"_hs).get(), true);
	registry.assign<TransformComponent>(plr, Transform());
	registry.assign<CameraComponent>(plr, Vector3f(), 0, 0);
	registry.assign<CameraDistanceComponent>(plr, 0.1f, 0.1f, 50.f);
	registry.assign<PlayerInputComponent>(plr);
	registry.assign<PlayerController>(plr, Vector3f(), 10.f);
	//registry.assign<Body>(plr, playerBody);
	registry.assign<CharacterController>(plr, playerCC);

	rot = Math::rotate(Quaternion(1, 0, 0, 0), Math::toRadians(0.f), Vector3f(0, 0, 1));

	//auto wallCollider = physics.createCollider<BoxCollider>(Vector3f(0.5f, 4.f, 2.f));
	//auto wallBody = physics.createBody(wallCollider, 1.f, Vector3f(), rot);

	//auto wall = registry.create();
	//registry.assign<TransformComponent>(wall, Transform(Vector3f(), rot, Vector3f(0.5f, 4.f, 2.f)));
	//registry.assign<StaticMesh>(wall, &rm.vertexArrays.handle("cube"_hs).get(),
	//		&rm.materials.handle("bricks2"_hs).get(), true);
	//registry.assign<Body>(wall, wallBody);

	//wallBody.setFriction(1.0);
	//playerBody.setFriction(1.0);

	Ocean::init(RenderContext::ref(), RenderSystem::ref().getCamera(),
			256, 256, 250, true, 6.f);
}

void TempScene::update(float deltaTime) {
	auto& registry = Registry::ref();
	auto& app = Application::ref();
	auto& renderer = RenderSystem::ref();
	auto& physicsEngine = PhysicsEngine::ref();

	app.pollEvents();

	updatePlayerController(registry, deltaTime);

	registry.view<TransformComponent, CharacterController>().each([&](auto& tfc, auto& cc) {
		auto btTf = cc.getGhostObject()->getWorldTransform();
		Physics::btToNativeTransform(tfc.transform, btTf);

		auto lv = cc.getController()->getLinearVelocity();

		//DEBUG_LOG_TEMP("%.2f, %.2f, %.2f", lv.x(), lv.y(), lv.z());
	});

	registry.view<TransformComponent, VehicleController>().each([&](auto& tfc, auto& vc) {
		auto btTf = vc.getController()->getGhostObject()->getWorldTransform();
		Physics::btToNativeTransform(tfc.transform, btTf);
	});

	//Transform tf;
	//planeBodyPtr.getCenterOfMassTransform(tf);

	//tf.setPosition(tf.getPosition() + Vector3f(0, 0, 1.f * deltaTime));

	//planeBodyPtr.setCenterOfMassTransform(tf);
	//planeBodyPtr.getHandle()->getMotionState()->setWorldTransform(btTransform(Physics::nativeToBtQuat(tf.getRotation()),
	//		Physics::nativeToBtVec3(tf.getPosition())));
	//planeBodyPtr.getHandle()->setLinearVelocity(btVector3(0, 0, 1));
	//planeBodyPtr.setToAwake();
	//planeBodyPtr.getHandle()->applyCentralForce(btVector3(0, 1.0 * 9.81, 0));

	physicsEngine.step(deltaTime);
	physicsEngine.writeTransformComponents(registry);

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
	Ocean::ref().update(renderer.getCamera(), deltaTime);
}

void TempScene::render() {
	auto& registry = Registry::ref();
	auto& renderer = RenderSystem::ref();
	auto& context = RenderContext::ref();

	auto& rm = ResourceManager::ref();

	auto font = rm.fonts.handle("font"_hs);

	String fpsText = "FPS:" + std::to_string(SceneManager::ref().getFPS());

	renderer.drawText(font, fpsText, 20, 560, Vector3f(0, 0, 0));

	//renderer.drawTextureQuad(Ocean::ref().getFFTData().getDisplacement(), Vector4f(50, 50, 0, 0), Vector4f(600, 600, 1, 1), Vector3f(1, 1, 1));

	renderStaticMeshes(registry, renderer);
	renderRiggedMeshes(registry, renderer);

	renderer.clear();

	renderer.flushStaticMeshes();
	renderer.flushRiggedMeshes();

	Ocean::ref().render();

	renderer.applyLighting();
	
	renderer.renderSkybox();

	renderer.flush();

	renderer.flushTexturedQuads();

	PhysicsEngine::ref().debugDrawWorld();

	Application::ref().swapBuffers();
}

void TempScene::unload() {
}
