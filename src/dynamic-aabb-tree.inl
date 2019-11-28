
inline void* DynamicAABBTree::getUserData(int32 id) const {
	return nodes[id].userData;
}

inline const AABB& DynamicAABBTree::getFatAABB(int32 id) const {
	return nodes[id].aabb;
}

template <typename T>
inline void DynamicAABBTree::query(T& callback, const AABB& aabb) const {
	ArrayList<int32> indexStack;

	indexStack.push_back(root);

	while (!indexStack.empty()) {
		int32 id = indexStack.back();
		indexStack.pop_back();

		const Node* n = nodes + id;

		if (aabb.intersects(n->aabb)) {
			if (n->isLeaf()) {
				if (!callback.treeCallback(id)) {
					return;
				}
			}
			else {
				indexStack.push_back(n->left);
				indexStack.push_back(n->right);
			}
		}
	}
}

