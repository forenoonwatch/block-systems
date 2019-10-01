#include "common.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

#define EXPOSURE 2.0

uniform sampler2D screen;

layout (location = 0) out vec4 outColor;

void main() {
	const float gamma = 2.2;
	const vec3 hdrColor = texelFetch(screen, ivec2(gl_FragCoord.xy), 0).rgb;

	//vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
	vec3 mapped = vec3(1.0) - exp(-hdrColor * EXPOSURE);

	//mapped = pow(mapped, vec3(1.0 / gamma));

	outColor = vec4(mapped, 1.0);
	//outColor = vec4(hdrColor, 1.0);
}

#endif

