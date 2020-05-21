#include "common.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

uniform sampler2D screen;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = texelFetch(screen, ivec2(gl_FragCoord.xy), 0);
}

#endif
