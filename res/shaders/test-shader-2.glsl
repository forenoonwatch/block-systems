#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

layout (std140) uniform UniformBufferTest {
    vec4 someVec4;
    vec2 someVec2;
    vec3 tableThing[4];
};

uniform vec3 freeUniform;
uniform mat4 freeUniform2;

void main() {
    gl_Position = vec4(position + freeUniform.xy + freeUniform2[0].xy, 0.0, 1.0);
}

#elif defined(FS_BUILD)

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0);
}

#endif