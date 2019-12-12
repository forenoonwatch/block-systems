#include "dynamic-aabb-tree.hpp"

#include <engine/core/memory.hpp>

#include <engine/math/math.hpp>

// TODO: comment all operations
// TODO: subdivide balance, add, and remove into requisite AVL tree ops

#define FATTENING_SCALAR	0.5f

#define DEFAULT_CAPACITY	1024

DynamicAABBTree::DynamicAABBTree()
		: root(Node::NULL_NODE)
		, capacity(DEFAULT_CAPACITY)
		, numNodes(0)
		, nodes(new Node[DEFAULT_CAPACITY]) {
	addToFreeList(0);
}

int32 DynamicAABBTree::insert(const AABB& aabb, void* userData) {
	int32 id = allocateNode();

	nodes[id].aabb = aabb.expand(FATTENING_SCALAR);
	nodes[id].userData = userData;
	nodes[id].depth = 0;

	insertLeaf(id);

	return id;
}

void DynamicAABBTree::remove(int32 id) {
	removeLeaf(id);
	freeNode(id);
}

bool DynamicAABBTree::update(int32 id, const AABB& aabb) {
	if (nodes[id].aabb.overlap(aabb).getVolume() < 0.0000001f) {
		return false;
	}

	removeLeaf(id);

	nodes[id].aabb = aabb.expand(FATTENING_SCALAR);

	insertLeaf(id);

	return true;
}

inline void DynamicAABBTree::insertLeaf(int32 id) {
	if (root == Node::NULL_NODE) {
		root = id;
		nodes[root].parent = Node::NULL_NODE;

		return;
	}

	int32 sibling = findSibling(id);

	int32 oldParent = nodes[sibling].parent;
	int32 newParent = allocateNode();

	nodes[newParent].parent = oldParent;
	nodes[newParent].userData = nullptr;
	nodes[newParent].aabb = nodes[id].aabb.addAABB(nodes[sibling].aabb);
	nodes[newParent].depth = nodes[sibling].depth + 1;

	if (oldParent == Node::NULL_NODE) {
		nodes[newParent].left = sibling;
		nodes[newParent].right = id;

		nodes[sibling].parent = newParent;
		nodes[id].parent = newParent;

		root = newParent;
	}
	else {
		if (nodes[oldParent].left == sibling) {
			nodes[oldParent].left = newParent;
		}
		else {
			nodes[oldParent].right = newParent;
		}

		nodes[newParent].left = sibling;
		nodes[newParent].right = id;

		nodes[sibling].parent = newParent;
		nodes[id].parent = newParent;
	}

	syncHierarchy(nodes[id].parent);
}

inline void DynamicAABBTree::removeLeaf(int32 id) {
	if (id == root) {
		root = Node::NULL_NODE;
		return;
	}

	int32 parent = nodes[id].parent;
	int32 grandparent = nodes[parent].parent;
	int32 sibling;

	if (nodes[parent].left == id) {
		sibling = nodes[parent].right;
	}
	else {
		sibling = nodes[parent].left;
	}

	if (grandparent != Node::NULL_NODE) {
		if (nodes[grandparent].left == parent) {
			nodes[grandparent].left = sibling;
		}
		else {
			nodes[grandparent].right = sibling;
		}

		nodes[sibling].parent = grandparent;
	}
	else {
		root = sibling;
		nodes[sibling].parent = Node::NULL_NODE;
	}

	freeNode(parent);
	syncHierarchy(grandparent);
}

inline int32 DynamicAABBTree::findSibling(int32 id) {
	int32 searchIndex = root;
	AABB leafAABB = nodes[id].aabb;

	while (!nodes[searchIndex].isLeaf()) {
		AABB combined = leafAABB.addAABB(nodes[searchIndex].aabb);
		float combinedArea = combined.getSurfaceArea();
		float branchCost = 2.f * combinedArea;

		float inheritedCost = 2.f * (combinedArea
				- nodes[searchIndex].aabb.getSurfaceArea());

		int32 left = nodes[searchIndex].left;
		int32 right = nodes[searchIndex].right;

		float leftDescentCost;
		float rightDescentCost;
		
		if (nodes[left].isLeaf()) {
			leftDescentCost
					= leafAABB.addAABB(nodes[left].aabb).getSurfaceArea()
					+ inheritedCost;
		}
		else {
			float inflated
					= leafAABB.addAABB(nodes[left].aabb).getSurfaceArea();
			float branchArea = nodes[left].aabb.getSurfaceArea();

			leftDescentCost = inflated - branchArea + inheritedCost;
		}

		if (nodes[right].isLeaf()) {
			rightDescentCost
					= leafAABB.addAABB(nodes[right].aabb).getSurfaceArea()
					+ inheritedCost;
		}
		else {
			float inflated
					= leafAABB.addAABB(nodes[right].aabb).getSurfaceArea();
			float branchArea = nodes[right].aabb.getSurfaceArea();

			rightDescentCost = inflated - branchArea + inheritedCost;
		}

		if (branchCost < leftDescentCost && branchCost < rightDescentCost) {
			break;
		}

		searchIndex = leftDescentCost < rightDescentCost ? left : right;
	}

	return searchIndex;
}

inline int32 DynamicAABBTree::balance(int32 iA) {
	Node* a = nodes + iA;

	if (a->isLeaf() || a->depth == 1) {
		return iA;
	}

	int32 iB = a->left;
	int32 iC = a->right;

	Node* b = nodes + iB;
	Node* c = nodes + iC;

	int32 bal = c->depth - b->depth;

	if (bal > 1) {
		int32 iF = c->left;
		int32 iG = c->right;

		Node* f = nodes + iF;
		Node* g = nodes + iG;

		if (a->parent != Node::NULL_NODE) {
			if (nodes[a->parent].left == iA) {
				nodes[a->parent].left = iC;
			}
			else {
				nodes[a->parent].right = iC;
			}
		}
		else {
			root = iC;
		}

		c->left = iA;
		c->parent = a->parent;
		a->parent = iC;

		if (f->depth > g->depth) {
			c->right = iF;
			a->right = iG;
			g->parent = iA;

			a->aabb = b->aabb.addAABB(g->aabb);
			c->aabb = a->aabb.addAABB(f->aabb);

			a->depth = 1 + Math::max(b->depth, g->depth);
			c->depth = 1 + Math::max(a->depth, f->depth);
		}
		else {
			c->right = iG;
			a->right = iF;
			f->parent = iA;

			a->aabb = b->aabb.addAABB(f->aabb);
			c->aabb = a->aabb.addAABB(g->aabb);

			a->depth = 1 + Math::max(b->depth, f->depth);
			c->depth = 1 + Math::max(a->depth, g->depth);
		}

		return iC;
	}
	else if (bal < -1) {
		int32 iD = b->left;
		int32 iE = b->right;

		Node* d = nodes + iD;
		Node* e = nodes + iE;

		if (a->parent != Node::NULL_NODE) {
			if (nodes[a->parent].left == iA) {
				nodes[a->parent].left = iB;
			}
			else {
				nodes[a->parent].right = iB;
			}
		}
		else {
			root = iB;
		}

		b->right = iA;
		b->parent = a->parent;
		a->parent = iB;

		if (d->depth > e->depth) {
			b->left = iD;
			a->left = iE;
			e->parent = iA;
			
			a->aabb = c->aabb.addAABB(e->aabb);
			b->aabb = a->aabb.addAABB(d->aabb);

			a->depth = 1 + Math::max(c->depth, e->depth);
			b->depth = 1 + Math::max(a->depth, d->depth);
		}
		else {
			b->left = iE;
			a->left = iD;
			d->parent = iA;

			a->aabb = c->aabb.addAABB(d->aabb);
			b->aabb = a->aabb.addAABB(e->aabb);

			a->depth = 1 + Math::max(c->depth, d->depth);
			b->depth = 1 + Math::max(a->depth, e->depth);
		}

		return iB;
	}

	return iA;
}

inline int32 DynamicAABBTree::allocateNode() {
	if (freeList == Node::NULL_NODE) {
		capacity *= 2;

		Node* data = new Node[capacity];
		Memory::memcpy(data, nodes, numNodes * sizeof(Node));
		delete[] nodes;
		nodes = data;

		addToFreeList(numNodes);
	}

	int32 fn = freeList;
	freeList = nodes[freeList].next;

	nodes[fn].depth = 0;
	nodes[fn].left = nodes[fn].right = nodes[fn].parent = Node::NULL_NODE;
	nodes[fn].userData = nullptr;

	++numNodes;

	return fn;
}

inline void DynamicAABBTree::freeNode(int32 index) {
	nodes[index].next = freeList;
	nodes[index].depth = Node::NULL_NODE;
	
	freeList = index;

	--numNodes;
}

inline void DynamicAABBTree::syncHierarchy(int32 index) {
	while (index != Node::NULL_NODE) {
		index = balance(index);

		int32 left = nodes[index].left;
		int32 right = nodes[index].right;

		nodes[index].depth = 1 + Math::max(nodes[left].depth,
				nodes[right].depth);
		nodes[index].aabb = nodes[left].aabb.addAABB(nodes[right].aabb);

		index = nodes[index].parent;
	}
}

inline void DynamicAABBTree::addToFreeList(int32 index) {
	for (int32 i = index; i < capacity - 1; ++i) {
		nodes[i].next = i + 1;
		nodes[i].depth = Node::NULL_NODE;
	}

	nodes[capacity - 1].next = Node::NULL_NODE;
	nodes[capacity - 1].depth = Node::NULL_NODE;

	freeList = index;
}

DynamicAABBTree::~DynamicAABBTree() {
	delete[] nodes;
}

