#pragma once

#include <engine/core/common.hpp>
#include <engine/math/aabb.hpp>

class DynamicAABBTree {
	public:
		DynamicAABBTree();

		int32 insert(const AABB& aabb, void* userData);
		void remove(int32 id);

		bool update(int32 id, const AABB& aabb);

		inline void* getUserData(int32 id) const;
		inline const AABB& getFatAABB(int32 id) const;

		~DynamicAABBTree();
	private:
		struct Node {
			static constexpr const int32 NULL_NODE = -1;

			inline bool isLeaf() const { return right == NULL_NODE; }
			
			AABB aabb;
			void* userData;
			int32 depth;
			int32 left;
			int32 right;

			union {
				int32 parent;
				int32 next;
			};
		};

		int32 root;

		Node* nodes;
		int32 numNodes;
		int32 capacity;

		int32 freeList;


		void insertLeaf(int32);
		void removeLeaf(int32);

		int32 findSibling(int32);
		
		int32 balance(int32);

		int32 allocateNode();
		void freeNode(int32);

		void syncHierarchy(int32);
		void addToFreeList(int32);
};

inline void* DynamicAABBTree::getUserData(int32 id) const {
	return nodes[id].userData;
}

inline const AABB& DynamicAABBTree::getFatAABB(int32 id) const {
	return nodes[id].aabb;
}

