#include "block.hpp"

#include <engine/rendering/indexed-model.hpp>
#include <engine/rendering/vertex-array.hpp>
#include <engine/rendering/material.hpp>

#include <engine/rendering/render-context.hpp>

#include <engine/game/game.hpp>

#include <engine/core/util.hpp>

#include <cstdlib>
#include <fstream>

ArrayList<BlockInfo> BlockInfo::blockInfo;

void BlockInfo::loadBlockInfo(Game& game, const String& fileName) {
	std::ifstream file(fileName);

	struct IndexedModel::AllocationHints hints;
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(2);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(3);
	hints.elementSizes.push_back(16);
	hints.instancedElementStartIndex = 4;

	String line;
	ArrayList<String> tokens;

	uint32 typeID = 0;

	// FORMAT: model-name,material-name,mass,volume,model-file-path
	// n = 5

	if (file.is_open()) {
		while (file.good()) {
			std::getline(file, line);

			tokens.clear();
			Util::split(tokens, line, ',');

			if (tokens.size() != 5) {
				continue;
			}

			game.getAssetManager().loadStaticMesh(tokens[0], tokens[0],
					tokens[4], hints);

			BlockInfo::registerType(typeID,
					0,
					&game.getAssetManager().getModel(tokens[0]),
					&game.getAssetManager().getMaterial(tokens[1]),
					std::atof(tokens[2].c_str()),
					std::atof(tokens[3].c_str()));
			
			++typeID;
		}
	}

	/*BlockInfo::registerType(BlockInfo::TYPE_BASIC_CUBE,
			BlockInfo::FLAG_OCCLUDES,
			&game.getAssetManager().getModel("cube"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.5f,//0.8f,
			1.f);
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_TETRA,
			0,
			&game.getAssetManager().getModel("tetrahedron"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.2f,
			0.2f);
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_PYRAMID,
			0,
			&game.getAssetManager().getModel("pyramid"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.4f,
			0.4f); // TODO: double check this mass
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE,
			0,
			&game.getAssetManager().getModel("wedge"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.5f,
			0.5f);
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_FIVE_SIXTH,
			0,
			&game.getAssetManager().getModel("five-sixths-block"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.8f,
			0.8f);
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE_2X1,
			0,
			&game.getAssetManager().getModel("wedge-2x-1"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.6f,
			0.6f); // TODO: calculate accurate mass
	BlockInfo::registerType(BlockInfo::TYPE_BASIC_WEDGE_2X1,
			0,
			&game.getAssetManager().getModel("wedge-2x-2"),
			&game.getAssetManager().getMaterial("wood-planks"),
			0.4f,
			0.4f); // TODO: calculate accurate mass*/
}

void BlockInfo::registerType(uint32 type, uint32 flags,
		IndexedModel* model, Material* material, float mass, float volume) {
	blockInfo.push_back(BlockInfo(type, flags, model, material, mass, volume));
}

void BlockInfo::initVertexArrays(RenderContext& renderContext,
		ArrayList<Memory::SharedPointer<VertexArray>>& vaos) {
	for (auto& info : blockInfo) {
		vaos.emplace_back(new VertexArray(renderContext, *info.model,
				GL_STATIC_DRAW));
	}
}

