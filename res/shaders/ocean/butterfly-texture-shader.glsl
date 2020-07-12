#include "../common.glh"
#include "../complex.glh"

#if defined(CS_BUILD)

layout (local_size_x = 1, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D butterflyTexture;

layout (std430, binding = 0) buffer indices {
	int v[];
} bitReversedIndices;

uniform int N;

void main() {
	const vec2 x = vec2(gl_GlobalInvocationID.xy);
	const float k = mod(x.y * (float(N) / pow(2, x.x + 1)), N);
	const complex twiddle = complex(cos(2.0 * M_PI * k / float(N)),
									sin(2.0 * M_PI * k / float(N)));
	
	const int span = int(pow(2, x.x));
	
	int wing;
	
	if (mod(x.y, pow(2, x.x + 1)) < pow(2, x.x)) {
		wing = 1;
	}
	else {
		wing = 0;
	}

	if (x.x == 0) {
		if (wing == 1) {
			imageStore(butterflyTexture, ivec2(x), vec4(twiddle, bitReversedIndices.v[int(x.y)],
					bitReversedIndices.v[int(x.y + 1)]));
		}
		else {
			imageStore(butterflyTexture, ivec2(x), vec4(twiddle, bitReversedIndices.v[int(x.y - 1)],
					bitReversedIndices.v[int(x.y)]));
		}
	}
	else {
		if (wing == 1) {
			imageStore(butterflyTexture, ivec2(x), vec4(twiddle, x.y, x.y + span));
		}
		else {
			imageStore(butterflyTexture, ivec2(x), vec4(twiddle, x.y - span, x.y));
		}
	}
}

#endif
