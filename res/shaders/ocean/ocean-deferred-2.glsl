#include "../common.glh"
#include "../bicubic-sampling.glh"

#include "../scene-info.glh"
#include "../lighting.glh"

#include "ocean-common.glh"

#define SSS_POWER 2.0

uniform sampler2D displacementMap;

vec3 oceanData(vec2 pos) {
	vec3 height = texture(displacementMap, pos).rgb * vec3(lambda, amplitude, lambda);
	return height;
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	const vec3 data = oceanData(o.xz / o.w * OCEAN_SAMPLE);

	o.xyz += data * o.w;

	return o;
}

vec2 getOceanUV(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);
	const vec4 o = mix(a, b, pos.y);

	return o.xz / o.w * OCEAN_SAMPLE;
}

varying vec2 xyPos0;

#if defined(VS_BUILD)

layout (location = 0) in vec2 xyPos;

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos);
	const vec4 vertPos = viewProjection * p0Raw;

	gl_Position = vertPos;

	xyPos0 = getOceanUV(xyPos);
}

#elif defined(FS_BUILD)

//const vec3 oceanColor0 = vec3(31, 71, 87) / 255.0;
const vec3 oceanColor0 = vec3(0, 20, 40) / 255.0;
const vec3 oceanColor1 = vec3(18, 125, 120) / 255.0;

uniform sampler2D foldingMap;
//uniform sampler2D foam;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec4 foldNorm = texture2D(foldingMap, xyPos0);
	const vec3 normal = foldNorm.xyz;
	const float folding = foldNorm.w;

	outColor = vec4(mix(oceanColor0, vec3(1.0), folding), 1.0);
	outNormal = vec4(normal, 1.0);
	outLighting = vec4(0, mix(0.15, 1.0, folding), mix(0.8, 0.0, min(folding, 1.0)), 1);

	/*float foamMask = foldNorm.w;

	//float foamMask = texture2D(foldingMap, getOceanUV(xyPos0)).w;//foamData(xyPos0);
	foamMask *= max(0.0, 1.1 - texture2D(foam, getOceanUV(xyPos0)).a);//foamData(xyPos0);

	const float shininess = 1.0 - foamMask;

	//foamMask *= texture2D(foam, 0.3 * p0.xz).y;
	
	const float sssFactor = clamp(SSS_POWER * (1.0 - normal.y), 0.0, 1.0)
			* max(-sunlightDir.y, 0.0);

	vec3 waterColor = mix(oceanColor0, oceanColor1, 0.2 * sssFactor);
	waterColor = mix(waterColor, vec3(1.0), foamMask);

	float metallic = 0.3;// 0.7 * (1.0 - sssFactor);
	metallic *= (1.0 - foamMask);

	// TODO: use this for translucency
	//float lightWeight = 1.0 - sssFactor;
	//lightWeight = mix(fma(lightWeight, 0.4, 0.6), 1.0, foamMask);

	outColor = vec4(waterColor, 1.0);
	outNormal = vec4(normal, 1.0);
	outLighting = vec4(metallic, 0.1 + 0.9 * foamMask, 0.5 * foamMask + 0.5, 1.0);*/
}

#endif
