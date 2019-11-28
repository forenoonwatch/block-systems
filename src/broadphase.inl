
inline bool Physics::Broadphase::treeCallback(int32 index) {
	if (index == currentIndex) {
		return true;
	}

	int32 iA, iB;

	if (index <= currentIndex) {
		iA = index;
		iB = currentIndex;
	}
	else {
		iA = currentIndex;
		iB = index;
	}

	pairBuffer.emplace_back(iA, iB);

	return true;
}

