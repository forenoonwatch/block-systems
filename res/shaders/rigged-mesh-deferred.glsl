#include "common.glh"
#include "scene-info.glh"

#define MAX_JOINTS 20

varying vec2 texCoord0;
varying mat3 TBN;

varying vec3 cameraPos0;
varying vec3 vertPos0;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;
layout (location = 5) in vec3 jointIndices;
layout (location = 6) in vec3 jointWeights;
layout (location = 7) in mat4 transform;

layout (std140) uniform AnimationData {
	mat4 jointTransforms[MAX_JOINTS];
};

void main() {
	mat4 jointTransform = jointTransforms[int(jointIndices[0])] * jointWeights[0];
	jointTransform += jointTransforms[int(jointIndices[1])] * jointWeights[1];
	jointTransform += jointTransforms[int(jointIndices[2])] * jointWeights[2];

	const mat4 model = transform * jointTransform;

	const vec4 vertPos = model * vec4(position, 1.0);

	vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
	vec3 B = -normalize(vec3(model * vec4(biTangent, 0.0)));
	T = normalize(T - dot(T, N) * N);

	gl_Position = viewProjection * vertPos;

	texCoord0 = texCoord;
	TBN = mat3(T, B, N);

	const mat3 invTBN = transpose(TBN);

	cameraPos0 = invTBN * cameraPosition;
	vertPos0 = invTBN * vec3(vertPos);
}

#elif defined(FS_BUILD)

#include "parallax-map.glh"

uniform sampler2D diffuse;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D depthMap;

uniform float heightScale;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec2 texCoords = parallaxMap(depthMap, texCoord0, normalize(cameraPos0 - vertPos0), heightScale);

	if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
		discard;
	}

	vec3 normal = fma(texture(normalMap, texCoords).rgb, vec3(2.0), vec3(-1.0));
	normal = TBN * normalize(normal);

	outColor = vec4(texture(diffuse, texCoords).rgb, 1.0);
	outNormal = vec4(normal, 1.0);
	outLighting = texture2D(materialMap, texCoords);
}

#endif
