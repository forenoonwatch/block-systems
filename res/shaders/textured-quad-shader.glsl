#include "common.glh"

varying vec2 texCoord0;
varying vec3 color0;

#if defined(VS_BUILD)

#include "scene-info.glh"

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 positions;
layout (location = 2) in vec4 sizes;
layout (location = 3) in vec3 color;

void main() {
    vec4 v = fma(vertex, sizes, positions);
    vec2 pos = v.xy / (displaySize * 0.5) - vec2(1);

    gl_Position = vec4(pos, 0.0, 1.0);
    texCoord0 = v.zw;
    color0 = color;
}

#elif defined(FS_BUILD)

uniform sampler2D diffuse;

layout (location = 0) out vec4 outColor;

void main() {
    const float c = texture2D(diffuse, texCoord0).r;

    outColor = vec4(color0 * c, c);
}

#endif