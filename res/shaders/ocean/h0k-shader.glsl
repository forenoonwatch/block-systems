#include "../common.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D imageH0k;

layout (binding = 1) uniform sampler2D noise_r0;
layout (binding = 2) uniform sampler2D noise_i0;
layout (binding = 3) uniform sampler2D noise_r1;
layout (binding = 4) uniform sampler2D noise_i1;

uniform	int N;
uniform int L;

uniform float amplitude;
uniform float intensity;
uniform vec2 direction;
uniform float l; // capillar suppression factor

vec4 gaussRND() {
	const vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / float(N);
	
	const float noise00 = clamp(texture(noise_r0, texCoord).r, 0.001, 1.0);
	const float noise01 = clamp(texture(noise_i0, texCoord).r, 0.001, 1.0);
	const float noise02 = clamp(texture(noise_r1, texCoord).r, 0.001, 1.0);
	const float noise03 = clamp(texture(noise_i1, texCoord).r, 0.001, 1.0);
	
	const float u0 = 2.0 * M_PI * noise00;
	const float v0 = sqrt(-2.0 * log(noise01));
	const float u1 = 2.0 * M_PI * noise02;
	const float v1 = sqrt(-2.0 * log(noise03));
	
	return vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));
}

void main() {
	const vec2 x = vec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	const vec2 k = x * (2.0 * M_PI / L);

	const float L_ = (intensity * intensity) / GRAVITY;
	const float magSq = max(dot(k, k), 0.0000001);

	const float n = sqrt((amplitude/(magSq*magSq))
			* exp(-(1.0 / (magSq * L_ * L_)))
			* exp(-magSq*pow(l,2.0))) / sqrt(2.0);
	
	const float h0K = clamp(n * pow(dot(normalize(k), direction), 2.0), -4000.0, 4000.0);
	const float h0MinusK = clamp(n * pow(dot(normalize(-k), direction), 2.0), -4000.0, 4000.0);
	
	const vec4 rnd = gaussRND();
	
	imageStore(imageH0k, ivec2(gl_GlobalInvocationID.xy), rnd * vec4(h0K, h0K, h0MinusK, h0MinusK));
}

#endif
