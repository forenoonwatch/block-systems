#include "scene-info.glh"

//varying vec2 texCoord0;
//varying mat3 TBN;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 transform;

out mat4 T_MVP;
out mat4 T_MVN;

out vec3 vertex_normal;

void main() {
	const mat4 mvp = viewProjection * transform;
	const vec4 vertPos = mvp * vec4(position, 1.0);

	T_MVP = mvp;
	T_MVN = transform;

	const vec3 N = normalize(vec3(transform * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
	T = normalize(T - dot(T, N) * N);

	gl_Position = vec4(position, 1.0);
    //gl_Position = vertPos;
    //texCoord0 = texCoord;
	//TBN = (mat3(T, cross(N, T), N));
	vertex_normal = normal;
}

#elif defined(GS_BUILD)

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in mat4 T_MVP[]; // Model View Projection Matrix
in mat4 T_MVN[];

const float length = 0.5;

in vec3 vertex_normal[];

out vec3 vertex_color;

void main() {
    vec3 normal = vertex_normal[0];

    vertex_color = abs(vec4(T_MVN[0] * vec4(vertex_normal[0], 0.0)).xyz);

    vec4 v0 = gl_in[0].gl_Position;
    gl_Position = T_MVP[0] * v0;
    EmitVertex();

    vec4 v1 = v0 + vec4(normal * length, 0.0);
    gl_Position = T_MVP[0] * v1;
    EmitVertex();

    EndPrimitive();
}

#elif defined(FS_BUILD)

in vec3 vertex_color;

layout (location = 0) out vec4 outColor;

//uniform sampler2D normalMap;

void main() {
	//vec3 normal = fma(texture(normalMap, texCoord0).rgb, vec3(2.0), vec3(-1.0));
	//normal = TBN * normalize(normal);

    //outColor = vec4(fma(normal, vec3(0.5), vec3(0.5)), 1.0);
	outColor = vec4(vertex_color, 1.0);
}

#endif