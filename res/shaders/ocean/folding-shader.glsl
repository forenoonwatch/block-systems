#include "../common.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) readonly uniform image2D displacement;
layout (binding = 1, rgba32f) uniform image2D foldingMap;

uniform int N;

uniform float lambda;
uniform float accum;
uniform float decay;

#define LAMBDA 1.0
#define BIAS 0.25//1.75

#define ACCUM 0.1
#define DECAY 0.8

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	//const vec2 d00 = imageLoad(displacement, x).xz;
	const vec2 d10 = imageLoad(displacement, ivec2( mod(x + vec2(1, 0), vec2(N)) )).xz;
	const vec2 dn10 = imageLoad(displacement, ivec2( mod(x + vec2(-1, 0), vec2(N)) )).xz;
	const vec2 d01 = imageLoad(displacement, ivec2( mod(x + vec2(0, 1), vec2(N)) )).xz;
	const vec2 dn01 = imageLoad(displacement, ivec2( mod(x + vec2(0, -1), vec2(N)) )).xz;

	const vec2 dx = (dn10 - d10) * lambda * BIAS;
	const vec2 dy = (dn01 - d01) * lambda * BIAS;

	float J = (1.0 + dx.x) * (1.0 + dy.y) - dx.y * dy.x;
	J = max(1.0 - J, 0.0);

	const float fPrev = imageLoad(foldingMap, x).x;
	imageStore(foldingMap, x, vec4(vec3(max(J * accum + fPrev - decay, 0.0)), 1.0));

	// incorrect, 'peak mapping' equation
	//imageStore(foldingMap, x, vec4(vec3(smoothstep(max(J, 0.0), 0.0, 1.0)), 1.0));

	// 'correct' equation
	//imageStore(foldingMap, x, vec4(vec3(max(1.0 - J, 0.0)), 1.0));
}

#endif
