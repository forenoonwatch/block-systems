#include "scene-info.glh"

varying vec3 color0;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

void main() {
    gl_Position = viewProjection * vec4(position, 1.0);
    color0 = color;
}

#elif defined(FS_BUILD)

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(color0, 1.0);
}

#endif