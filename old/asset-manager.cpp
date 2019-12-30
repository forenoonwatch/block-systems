#include "engine/game/asset-manager.hpp"

#include "engine/rendering/vertex-array.hpp"
#include "engine/rendering/shader.hpp"
#include "engine/rendering/material.hpp"

#include "engine/core/util.hpp"
#include "engine/core/asset-loader.hpp"

bool AssetManager::loadStaticMesh(const String& name, const String& fileName,
		const IndexedModel::AllocationHints& hints) {
	ArrayList<IndexedModel> indexedModels;

	if (!AssetLoader::loadAssets(fileName, hints, indexedModels)) {
		DEBUG_LOG(LOG_ERROR, "Static Mesh",
				"Failed to load static mesh data from: %s", fileName.c_str());
		return false;
	}

	Memory::SharedPointer<VertexArray> va = Memory::make_shared<VertexArray>(
			*renderContext, indexedModels[0], GL_STATIC_DRAW);

	vertexArrays[name] = va;
	names[(uintptr)va.get()] = String("VertexArray/") + name;

	return true;
}

bool AssetManager::loadStaticMesh(const String& name, const String& modelName,
		const String& fileName, const IndexedModel::AllocationHints& hints) {
	ArrayList<IndexedModel> indexedModels;

	if (!AssetLoader::loadAssets(fileName, hints, indexedModels)) {
		DEBUG_LOG(LOG_ERROR, "Static Mesh",
				"Failed to load static mesh data from: %s", fileName.c_str());
		return false;
	}

	Memory::SharedPointer<VertexArray> va = Memory::make_shared<VertexArray>(
			*renderContext, indexedModels[0], GL_STATIC_DRAW);
	Memory::SharedPointer<IndexedModel> im =
		Memory::make_shared<IndexedModel>(indexedModels[0]);

	vertexArrays[name] = va;
	models[modelName] = im;

	names[(uintptr)va.get()] = String("VertexArray/") + name;
	names[(uintptr)im.get()] = String("IndexedModel/") + modelName;

	return true;
}

bool AssetManager::loadTexture(const String& name, const String& fileName) {
	const String fileExtension = Util::getFileExtension(fileName).to_lower();

	if (fileExtension.compare("dds") == 0) {
		DDSTexture ddsTexture;

		if (!ddsTexture.load(fileName)) {
			return false;
		}

		Memory::SharedPointer<Texture> tx = Memory::make_shared<Texture>(
				*renderContext, ddsTexture);

		textures[name] = tx;
		names[(uintptr)tx.get()] = String("Texture/") + name;

		return true;
	}
	else if (fileExtension.compare("hdr") == 0) {
		DEBUG_LOG(LOG_ERROR, "Texture", "HDR Textures are not yet supported");

		return false;
	}
	else {
		Bitmap bmp;

		if (!bmp.load(fileName)) {
			return false;
		}

		Memory::SharedPointer<Texture> tx = Memory::make_shared<Texture>(
				*renderContext, bmp, GL_RGBA);

		textures[name] = tx;
		names[(uintptr)tx.get()] = String("Texture/") + name;

		return true;
	}
}

bool AssetManager::loadCubeMap(const String& name, const String* fileNames) {
	Memory::SharedPointer<CubeMap> cm = Memory::make_shared<CubeMap>(
			*renderContext);

	if (!cm->load(fileNames)) {
		return false;
	}

	cubeMaps[name] = cm;
	names[(uintptr)cm.get()] = String("CubeMap/") + name;

	return true;
}

bool AssetManager::loadCubeMap(const String& name, const String& fileName) {
	Memory::SharedPointer<CubeMap> cm = Memory::make_shared<CubeMap>(
			*renderContext);

	if (!cm->load(fileName)) {
		return false;
	}

	cubeMaps[name] = cm;
	names[(uintptr)cm.get()] = String("CubeMap/") + name;

	return true;
}

bool AssetManager::loadMaterial(const String& name, const String& diffuseFileName,
		const String& normalFileName, const String& materialFileName) {
	if (!loadTexture(name + "-diffuse", diffuseFileName)) {
		return false;
	}

	if (!loadTexture(name + "-normal", normalFileName)) {
		return false;
	}

	if (!loadTexture(name + "-material", materialFileName)) {
		return false;
	}

	Memory::SharedPointer<Material> mt = Memory::make_shared<Material>();
	mt->diffuse = &getTexture(name + "-diffuse");
	mt->normalMap = &getTexture(name + "-normal");
	mt->materialMap = &getTexture(name + "-material");

	materials[name] = mt;
	names[(uintptr)mt.get()] = String("Material/") + name;

	return true;
}

bool AssetManager::loadShader(const String& name, const String& fileName) {
	Memory::SharedPointer<Shader> sh = Memory::make_shared<Shader>(
			*renderContext);

	if (!sh->load(fileName)) {
		return false;
	}

	shaders[name] = sh;
	names[(uintptr)sh.get()] = String("Shader/") + name;

	return true;
}
