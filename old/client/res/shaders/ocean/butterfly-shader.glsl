#include "../common.glh"
#include "../complex.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) readonly uniform image2D butterflyTexture;

layout (binding = 1, rgba32f) uniform image2D buffer0;
layout (binding = 2, rgba32f) uniform image2D buffer1;

uniform int direction;
uniform int bufferNum;
uniform int stage;

void horizontalButterflies() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	const vec4 data = imageLoad(butterflyTexture, ivec2(stage, x.x));

	if (bufferNum == 0) {
		const complex p = imageLoad(buffer0, ivec2(data.z, x.y)).rg;
		const complex q = imageLoad(buffer0, ivec2(data.w, x.y)).rg;
		
		imageStore(buffer1, x, vec4(p + cmul(data.xy, q), 0, 1));
	}
	else {
		const complex p = imageLoad(buffer1, ivec2(data.z, x.y)).rg;
		const complex q = imageLoad(buffer1, ivec2(data.w, x.y)).rg;
		
		imageStore(buffer0, x, vec4(p + cmul(data.xy, q), 0, 1));
	}
}

void verticalButterflies() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	const vec4 data = imageLoad(butterflyTexture, ivec2(stage, x.y));

	if (bufferNum == 0) {
		const complex p = imageLoad(buffer0, ivec2(x.x, data.z)).rg;
		const complex q = imageLoad(buffer0, ivec2(x.x, data.w)).rg;
		
		imageStore(buffer1, x, vec4(p + cmul(data.xy, q), 0, 1));
	}
	else {
		const complex p = imageLoad(buffer1, ivec2(x.x, data.z)).rg;
		const complex q = imageLoad(buffer1, ivec2(x.x, data.w)).rg;
		
		imageStore(buffer0, x, vec4(p + cmul(data.xy, q), 0, 1));
	}
}

void main() {
	if (direction == 0) {
		horizontalButterflies();
	}
	else {
		verticalButterflies();
	}
}

#endif
