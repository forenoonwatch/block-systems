#include "../common.glh"
#include "../complex.glh"

#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D coeffDY;
layout (binding = 1, rgba32f) writeonly uniform image2D coeffDX;
layout (binding = 2, rgba32f) writeonly uniform image2D coeffDZ;

layout (binding = 3, rgba32f) readonly uniform image2D imageH0k;

uniform int N;
uniform int L;
uniform float t;

void main() {
	const vec2 x = ivec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	const vec2 k = x * (2.0 * M_PI / L);
	
	const float magnitude = max(length(k), 0.00001);
	const float w = sqrt(GRAVITY * magnitude);
	
	const vec4 h0k = imageLoad(imageH0k, ivec2(gl_GlobalInvocationID.xy));

	const complex fourierAmp = h0k.xy;
	const complex fourierAmpConj = conj(h0k.zw);
		
	const complex expIWT = complex(cos(w * t), sin(w * t));
	const complex invExpIWT = complex(expIWT.x, -expIWT.y);
	
	const complex hktDY = cmul(fourierAmp, expIWT) + cmul(fourierAmpConj, invExpIWT);
	const complex hktDX = cmul(complex(0.0, -k.x / magnitude), hktDY);
	const complex hktDZ = cmul(complex(0.0, -k.y / magnitude), hktDY);
	
	imageStore(coeffDY, ivec2(gl_GlobalInvocationID.xy), vec4(hktDY, 0, 1));
	imageStore(coeffDX, ivec2(gl_GlobalInvocationID.xy), vec4(hktDX, 0, 1));
	imageStore(coeffDZ, ivec2(gl_GlobalInvocationID.xy), vec4(hktDZ, 0, 1));
}

#endif
