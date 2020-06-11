#include "common.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

uniform sampler2D image;

uniform bool horizontal;

const float weight[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

layout (location = 0) out vec4 outColor;

void main() {
	const ivec2 x = ivec2(gl_FragCoord.xy);
	vec3 result = texelFetch(image, x, 0).rgb * weight[0];

	if (horizontal) {
		for (int i = 1; i < 5; ++i) {
			result += texelFetch(image, x + ivec2(i, 0), 0).rgb * weight[i];
			result += texelFetch(image, x - ivec2(i, 0), 0).rgb * weight[i];
		}
	}
	else {
		for (int i = 1; i < 5; ++i) {
			result += texelFetch(image, x + ivec2(0, i), 0).rgb * weight[i];
			result += texelFetch(image, x - ivec2(0, i), 0).rgb * weight[i];
		}
	}
	
	outColor = vec4(result, 1.0);
}

#endif
