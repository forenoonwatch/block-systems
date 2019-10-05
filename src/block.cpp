#include "block.hpp"

ArrayList<BlockInfo> BlockInfo::blockInfo;

void BlockInfo::registerType(enum BlockType type, const Vector3i& occupancy,
		IndexedModel& model, VertexArray& vertexArray, Material& material) {
	blockInfo.push_back(BlockInfo(type, occupancy, &model, &vertexArray, &material));
}

