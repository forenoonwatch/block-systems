#include "distance-gjk.hpp"

#include <cfloat>

#define GJK_MAX_ITERATIONS 20
#define EPSILON 1e-5f

namespace {
	struct SimplexVertex {
		Vector3f sA;
		Vector3f sB;
		Vector3f p;

		float u;
		uint32 iA;
		uint32 iB;
	};

	class Simplex {
		public:
			inline Simplex(const Vector3f& sA, const Vector3f& sB);

			inline Vector3f getSearchDirection() const;
			inline Vector3f getClosestPoint() const;

			inline void calcWitnessPoints(Vector3f& a, Vector3f& b) const;

			inline uint32 solve();

			inline bool isDuplicate(uint32 iA, uint32 iB) const;

			inline void addVertex(const Vector3f& sA, const Vector3f& sB,
					uint32 iA, uint32 iB);

			inline uint32 getNumVertices() const { return numVertices; }
		private:
			SimplexVertex vertices[4];
			uint32 numVertices;

			float divisor;

			uint32 prevIndicesA[4];
			uint32 prevIndicesB[4];
			uint32 numPrevIndices;

			inline void solve2();
			inline void solve3();
			inline void solve4();
	};

	inline uint32 getSupportIndex(const Vector3f* vertices, uint32 numVertices,
			const Vector3f& normal);
};

void distanceGJK(const Vector3f* verticesA, uint32 numVerticesA,
		const Vector3f* verticesB, uint32 numVerticesB,
		const Transform& tfA, const Transform& tfB, Vector3f& outA,
		Vector3f& outB) {
	Simplex simplex(tfA.transform(verticesA[0], 1.f),
			tfB.transform(verticesB[0], 1.f));

	float dsq0 = FLT_MAX;
	float dsq1 = FLT_MAX;

	for (uint32 j = 0; j < GJK_MAX_ITERATIONS; ++j) {
		if (simplex.solve() == 4) {
			break;
		}

		Vector3f p = simplex.getClosestPoint();
		dsq1 = Math::dot(p, p);

		if (dsq1 > dsq0) {
			break;
		}

		dsq0 = dsq1;

		Vector3f d = simplex.getSearchDirection();

		if (Math::dot(d, d) < EPSILON * EPSILON) {
			break;
		}

		uint32 iA = getSupportIndex(verticesA, numVerticesA,
				tfA.inverseTransform(-d, 0.f));
		uint32 iB = getSupportIndex(verticesB, numVerticesB,
				tfB.inverseTransform(d, 0.f));

		if (simplex.isDuplicate(iA, iB)) {
			break;
		}

		Vector3f sA = tfA.transform(verticesA[iA], 1.f);
		Vector3f sB = tfB.transform(verticesB[iB], 1.f);

		simplex.addVertex(sA, sB, iA, iB);
	}

	simplex.calcWitnessPoints(outA, outB);
}

namespace {
	inline uint32 getSupportIndex(const Vector3f* vertices, uint32 numVertices,
				const Vector3f& normal) {
		float maxD = Math::dot(vertices[0], normal);
		uint32 maxI = 0;

		for (int32 i = 1; i < numVertices; ++i) {
			float d = Math::dot(vertices[i], normal);

			if (d > maxD) {
				maxD = d;
				maxI = i;
			}
		}

		return maxI;
	}
};

inline Simplex::Simplex(const Vector3f& sA, const Vector3f& sB)
		: numVertices(1) {
	vertices[0].iA = 0;
	vertices[0].iB = 0;

	vertices[0].sA = sA;
	vertices[0].sB = sB;
	vertices[0].p = sB - sA;

	vertices[0].u = 1.f;
}

inline Vector3f Simplex::getSearchDirection() const {
	switch (numVertices) {
		case 1:
			return -vertices[0].p;
		case 2:
		{
			Vector3f ba = vertices[0].p - vertices[1].p;
			return Math::cross(Math::cross(ba, -vertices[0].p), ba);
		}
		case 3:
		{
			Vector3f n = Math::cross(vertices[1].p - vertices[0].p,
					vertices[2].p - vertices[0].p);

			if (Math::dot(n, vertices[0].p) <= 0.f) {
				return n;
			}
			else {
				return -n;
			}
		}
		default:
			return Vector3f();
	}
}

inline Vector3f Simplex::getClosestPoint() const {
	float denom = 1.f / divisor;

	switch (numVertices) {
		case 1:
			return vertices[0].p;
		case 2:
			return vertices[0].p * (denom * vertices[0].u)
					+ vertices[1].p * (denom * vertices[1].u);
		case 3:
			return vertices[0].p * (denom * vertices[0].u)
					+ vertices[1].p * (denom * vertices[1].u)
					+ vertices[2].p * (denom * vertices[2].u);
		case 4:
			return vertices[0].p * (denom * vertices[0].u)
					+ vertices[1].p * (denom * vertices[1].u)
					+ vertices[2].p * (denom * vertices[2].u)
					+ vertices[3].p * (denom * vertices[3].u);
		default:
			return Vector3f();
	};
}

inline void Simplex::calcWitnessPoints(Vector3f& a, Vector3f& b) const {
	float denom = 1.f / divisor;

	switch (numVertices) {
		case 1:
			a = vertices[0].sA;
			b = vertices[0].sB;
			break;
		case 2:
			a = vertices[0].sA * (denom * vertices[0].u)
					+ vertices[1].sA * (denom * vertices[1].u);
			b = vertices[0].sB * (denom * vertices[0].u)
					+ vertices[1].sB * (denom * vertices[1].u);
			break;
		case 3:
			a = vertices[0].sA * (denom * vertices[0].u)
					+ vertices[1].sA * (denom * vertices[1].u)
					+ vertices[2].sA * (denom * vertices[2].u);
			b = vertices[0].sB * (denom * vertices[0].u)
					+ vertices[1].sB * (denom * vertices[1].u)
					+ vertices[2].sB * (denom * vertices[2].u);
			break;
		case 4:
			a = vertices[0].sA * (denom * vertices[0].u)
					+ vertices[1].sA * (denom * vertices[1].u)
					+ vertices[2].sA * (denom * vertices[2].u)
					+ vertices[3].sA * (denom * vertices[3].u);
			b = a;
			break;
	}
}

inline uint32 Simplex::solve() {
	numPrevIndices = numVertices;

	for (uint32 i = 0; i < numPrevIndices; ++i) {
		prevIndicesA[i] = vertices[i].iA;
		prevIndicesB[i] = vertices[i].iB;
	}

	switch (numVertices) {
		case 1:
			break;
		case 2:
			solve2();
			break ;
		case 3:
			solve3();
			break;
		case 4:
			solve4();
			break;
	}

	return numVertices;
}

inline bool Simplex::isDuplicate(uint32 iA, uint32 iB) const {
	for (uint32 i = 0; i < numPrevIndices; ++i) {
		if (iA == prevIndicesA[i] && iB == prevIndicesB[i]) {
			return true;
		}
	}

	return false;
}

inline void Simplex::addVertex(const Vector3f& sA, const Vector3f& sB,
		uint32 iA, uint32 iB) {
	SimplexVertex* vertex = vertices + numVertices;
	vertex->iA = iA;
	vertex->sA = sA;
	vertex->iB = iB;
	vertex->sB = sB;
	vertex->p = vertex->sB - vertex->sA;

	++numVertices;
}

inline void Simplex::solve2() {
	Vector3f a = vertices[0].p;
	Vector3f b = vertices[1].p;

	float u = Math::dot(b, b - a);
	float v = Math::dot(a, a - b);

	// Region A
	if (v <= 0.f) {
		vertices[0].u = 1.f;
		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region B
	if (u <= 0.f) {
		vertices[0] = vertices[1];
		vertices[0].u = 1.f;
		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region AB
	vertices[0].u = u;
	vertices[1].u = v;
	divisor = u + v;
	numVertices = 2;
}

inline void Simplex::solve3() {
	Vector3f a = vertices[0].p;
	Vector3f b = vertices[1].p;
	Vector3f c = vertices[2].p;

	float uAB = Math::dot(b, b - a);
	float vAB = Math::dot(a, a - b);

	float uBC = Math::dot(c, c - b);
	float vBC = Math::dot(b, b - c);

	float uCA = Math::dot(a, a - c);
	float vCA = Math::dot(c, c - a);

	Vector3f n = Math::cross(b - a, c - a);

	float uABC = Math::dot(Math::cross(b, c), n);
	float vABC = Math::dot(Math::cross(c, a), n);
	float wABC = Math::dot(Math::cross(a, b), n);

	// Region A
	if (vAB <= 0.f && uCA <= 0.f) {
		vertices[0].u = 1.f;
		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region B
	if (uAB <= 0.f && vBC <= 0.f) {
		vertices[0] = vertices[1];

		vertices[0].u = 1.f;
		divisor = 1.f;

		numVertices = 1;

		return;
	}

	// Region C
	if ( uBC <= 0.f && vCA <= 0.f ) {
		vertices[0] = vertices[2];
		vertices[0].u = 1.f;

		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region AB
	if (uAB > 0.f && vAB > 0.f && wABC <= 0.f) {
		vertices[0].u = uAB;
		vertices[1].u = vAB;

		divisor = uAB + vAB;
		numVertices = 2;

		return;
	}

	// Region BC
	if (uBC > 0.f && vBC > 0.f && uABC <= 0.f) {
		vertices[0] = vertices[1];
		vertices[1] = vertices[2];

		vertices[0].u = uBC;
		vertices[1].u = vBC;

		divisor = uBC + vBC;
		numVertices = 2;

		return;
	}

	// Region CA
	if (uCA > 0.f && vCA > 0.f && vABC <= 0.f) {
		vertices[1] = vertices[0];
		vertices[0] = vertices[2];

		vertices[0].u = uCA;
		vertices[1].u = vCA;

		divisor = uCA + vCA;
		numVertices = 2;

		return;
	}

	// Region ABC
	vertices[0].u = uABC;
	vertices[1].u = vABC;
	vertices[2].u = wABC;

	divisor = uABC + vABC + wABC;
	numVertices = 3;
}

inline void Simplex::solve4() {
	Vector3f a = vertices[0].p;
	Vector3f b = vertices[1].p;
	Vector3f c = vertices[2].p;
	Vector3f d = vertices[3].p;

	float uAB = Math::dot(b, b - a);
	float vAB = Math::dot(a, a - b);

	float uBC = Math::dot(c, c - b);
	float vBC = Math::dot(b, b - c);

	float uCA = Math::dot(a, a - c);
	float vCA = Math::dot(c, c - a);

	float uBD = Math::dot(d, d - b);
	float vBD = Math::dot(b, b - d);

	float uDC = Math::dot(c, c - d);
	float vDC = Math::dot(d, d - c);

	float uAD = Math::dot(d, d - a);
	float vAD = Math::dot(a, a - d);

	Vector3f n = Math::cross(d - a, b - a);
	float uADB = Math::dot(Math::cross(d, b), n);
	float vADB = Math::dot(Math::cross(b, a), n);
	float wADB = Math::dot(Math::cross(a, d), n);

	n = Math::cross(c - a, d - a);
	float uACD = Math::dot(Math::cross(c, d), n);
	float vACD = Math::dot(Math::cross(d, a), n);
	float wACD = Math::dot(Math::cross(a, c), n);

	n = Math::cross(b - c, d - c);
	float uCBD = Math::dot(Math::cross(b, d), n);
	float vCBD = Math::dot(Math::cross(d, c), n);
	float wCBD = Math::dot(Math::cross(c, b), n);

	n = Math::cross(b - a, c - a);
	float uABC = Math::dot(Math::cross(b, c), n);
	float vABC = Math::dot(Math::cross(c, a), n);
	float wABC = Math::dot(Math::cross(a, b), n);

	float denom = Math::dot(c - b, Math::cross(a - b, d - b));
	float volume = denom == 0.f ? 1.f : 1.f / denom;

	float uABCD = Math::dot(c, Math::cross(d, b)) * volume;
	float vABCD = Math::dot(c, Math::cross(a, d)) * volume;
	float wABCD = Math::dot(d, Math::cross(a, b)) * volume;
	float xABCD = Math::dot(b, Math::cross(a, c)) * volume;

	// Region A
	if (vAB <= 0.f && uCA <= 0.f && vAD <= 0.f) {
		vertices[0].u = 1.f;

		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region B
	if (uAB <= 0.f && vBC <= 0.f && vBD <= 0.f) {
		vertices[0] = vertices[1];
		vertices[0].u = 1.f;

		divisor = 1.f;
		numVertices = 1;
		
		return;
	}

	// Region C
	if (uBC <= 0.f && vCA <= 0.f && uDC <= 0.f) {
		vertices[0] = vertices[2];
		vertices[0].u = 1.f;

		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region D
	if (uBD <= 0.f && vDC <= 0.f && uAD <= 0.f) {
		vertices[0] = vertices[3];
		vertices[0].u = 1.f;

		divisor = 1.f;
		numVertices = 1;

		return;
	}

	// Region AB
	if (wABC <= 0.f && vADB <= 0.f && uAB > 0.f && vAB > 0.f) {
		vertices[0].u = uAB;
		vertices[1].u = vAB;

		divisor = uAB + vAB;
		numVertices = 2;

		return;
	}

	// Region BC
	if (uABC <= 0.f && wCBD <= 0.f && uBC > 0.f && vBC > 0.f) {
		vertices[0] = vertices[1];
		vertices[1] = vertices[2];

		vertices[0].u = uBC;
		vertices[1].u = vBC;

		divisor = uBC + vBC;
		numVertices = 2;

		return;
	}

	// Region CA
	if (vABC <= 0.f && wACD <= 0.f && uCA > 0.f && vCA > 0.f) {
		vertices[1] = vertices[0];
		vertices[0] = vertices[2];

		vertices[0].u = uCA;
		vertices[1].u = vCA;

		divisor = uCA + vCA;
		numVertices = 2;

		return;
	}

	// Region DC
	if (vCBD <= 0.f && uACD <= 0.f && uDC > 0.f && vDC > 0.f) {
		vertices[0] = vertices[3];
		vertices[1] = vertices[2];

		vertices[0].u = uDC;
		vertices[1].u = vDC;

		divisor = uDC + vDC;
		numVertices = 2;

		return;
	}

	// Region AD
	if (vACD <= 0.f && wADB <= 0.f && uAD > 0.f && vAD > 0.f) {
		vertices[1] = vertices[3];

		vertices[0].u = uAD;
		vertices[1].u = vAD;

		divisor = uAD + vAD;
		numVertices = 2;

		return;
	}

	// Region BD
	if (uCBD <= 0.f && uADB <= 0.f && uBD > 0.f && vBD > 0.f) {
		vertices[0] = vertices[1];
		vertices[1] = vertices[3];

		vertices[0].u = uBD;
		vertices[1].u = vBD;

		divisor = uBD + vBD;
		numVertices = 2;

		return;
	}

	// Region ABC
	if (xABCD <= 0.f && uABC > 0.f && vABC > 0.f && wABC > 0.f) {
		vertices[0].u = uABC;
		vertices[1].u = vABC;
		vertices[2].u = wABC;

		divisor = uABC + vABC + wABC;
		numVertices = 3;

		return;
	}

	// Region CBD
	if (uABCD <= 0.f && uCBD > 0.f && vCBD > 0.f && vCBD > 0.f) {
		vertices[0] = vertices[2];
		vertices[2] = vertices[3];

		vertices[0].u = uCBD;
		vertices[1].u = vCBD;
		vertices[2].u = wCBD;

		divisor = uCBD + vCBD + wCBD;
		numVertices = 3;

		return;
	}

	// Region ACD
	if (vABCD <= 0.f && uACD > 0.f && vACD > 0.f && wACD > 0.f) {
		vertices[1] = vertices[2];
		vertices[2] = vertices[3];

		vertices[0].u = uACD;
		vertices[1].u = vACD;
		vertices[2].u = wACD;

		divisor = uACD + vACD + wACD;
		numVertices = 3;

		return;
	}

	// Region ADB
	if (wABCD <= 0.f && uADB > 0.f && vADB > 0.f && wADB > 0.f) {
		vertices[2] = vertices[1];
		vertices[1] = vertices[3];
		
		vertices[0].u = uADB;
		vertices[1].u = vADB;
		vertices[2].u = wADB;
		
		divisor = uADB + vADB + wADB;
		numVertices = 3;

		return;
	}

	// Region ABCD
	vertices[0].u = uABCD;
	vertices[1].u = vABCD;
	vertices[2].u = wABCD;
	vertices[3].u = xABCD;

	divisor = 1.f;
	numVertices = 4;
}

