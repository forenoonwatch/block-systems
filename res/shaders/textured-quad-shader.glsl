#include "common.glh"

varying vec2 texCoord0;

#if defined(VS_BUILD)

#include "scene-info.glh"

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec4 positions;
layout (location = 2) in vec4 sizes;

void main() {
    vec4 v = fma(vec4(vertex, vertex), sizes, positions);

    gl_Position = vec4(v.xy / displaySize - vec2(0.5), 0.0, 1.0);
    texCoord0 = v.zw;
}

#elif defined(FS_BUILD)

uniform sampler2D diffuse;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture2D(diffuse, texCoord0);
}

#endif