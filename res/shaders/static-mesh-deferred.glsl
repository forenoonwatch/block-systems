#include "common.glh"
#include "scene-info.glh"

varying vec2 texCoord0;
varying mat3 TBN;

varying vec3 cameraPos0;
varying vec3 vertPos0;

#if defined(VS_BUILD)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 transform;

void main() {
	const vec4 vertPos = transform * vec4(position, 1.0);

	const vec3 N = normalize(vec3(transform * vec4(normal, 0.0)));
	vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	gl_Position = viewProjection * vertPos;
	
	texCoord0 = texCoord;
	TBN = mat3(T, B, N);

	const mat3 invTBN = transpose(TBN);

	cameraPos0 = invTBN * cameraPosition;
	vertPos0 = invTBN * vec3(vertPos);
}

#elif defined(FS_BUILD)

#define HEIGHT_SCALE 0.1

uniform sampler2D diffuse;
uniform sampler2D normalMap;
uniform sampler2D materialMap;
uniform sampler2D depthMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

/*vec2 parallaxMap_simple(vec2 texCoords, vec3 viewDir) {
	const float height = texture(depthMap, texCoords).r;
	const vec2 p = viewDir.xy / viewDir.z * (height * HEIGHT_SCALE);

	return texCoords - p;
}*/

vec2 parallaxMap(vec2 texCoords, vec3 viewDir) {
	const float minLayers = 8;
    const float maxLayers = 32;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * HEIGHT_SCALE; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while (currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {
	const vec2 texCoords = parallaxMap(texCoord0, normalize(cameraPos0 - vertPos0));

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
