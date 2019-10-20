#include "../common.glh"
#include "../bicubic-sampling.glh"

#include "../scene-info.glh"
#include "../lighting.glh"

#include "ocean-common.glh"

#define F0 0.1 //0.017 // F0 = (n1 - n2) / (n1 + n2); n1 = 1, n2 = 1.3
#define SSS_POWER 2.0

uniform sampler2D displacementMap;

vec3 oceanData(vec2 pos) {
	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	vec3 height = textureBicubic(displacementMap, uv00, 1.0 / texelSize, frac)
			 * vec3(lambda, amplitude, lambda);

	height.y += texture2D(displacementMap,
			DETAIL_SAMPLE_MODIFIER * pos).y * detailAmplitude;

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

varying vec2 xyPos0;

varying float fresnel;

#if defined(VS_BUILD)

layout (location = 0) in vec2 xyPos;

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos);
	const vec4 p1Raw = getOceanPosition(xyPos + vec2(1.0 / (GRID_LENGTH - 1.0), 0.0));
	const vec4 p2Raw = getOceanPosition(xyPos + vec2(0.0, 1.0 / (GRID_LENGTH - 1.0)));

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 normal = normalize(cross(p2 - p0, p1 - p0));
	const vec4 vertPos = viewProjection * p0Raw;

	gl_Position = vertPos;

	xyPos0 = xyPos;

	const float cameraDist = length(cameraPosition - p0);
	
	const float F = clamp(1.0 - dot(normal, normalize(cameraPosition - p0)), 0.0, 1.0);

	fresnel = F * (max(clamp(1.0 - exp(-pow(cameraDist * 0.02, 1)), 0.0, 1.0),
			1.0 - normal.y) * 0.9 + 0.1);
}

#elif defined(FS_BUILD)

const vec3 oceanColor0 = vec3(31, 71, 87) / 255.0;
const vec3 oceanColor1 = vec3(18, 125, 120) / 255.0;

uniform sampler2D foldingMap;
uniform sampler2D foam;

float foamData(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);
	const vec4 o = mix(a, b, pos.y);

	pos = o.xz / o.w * OCEAN_SAMPLE;

	const vec2 uv00 = floor(pos * texelSize) / texelSize;
	const vec2 frac = vec2(pos - uv00) * texelSize;

	return clamp(textureBicubic(foldingMap, uv00, 1.0 / texelSize, frac).y, 0.0, 1.0);
}

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outLighting;

void main() {
	const vec4 p0Raw = getOceanPosition(xyPos0);
	const vec4 p1Raw = getOceanPosition(xyPos0 + vec2(1.0 / (GRID_LENGTH - 1.0), 0.0));
	const vec4 p2Raw = getOceanPosition(xyPos0 + vec2(0.0, 1.0 / (GRID_LENGTH - 1.0)));

	const vec3 p0 = p0Raw.xyz / p0Raw.w;
	const vec3 p1 = p1Raw.xyz / p1Raw.w;
	const vec3 p2 = p2Raw.xyz / p2Raw.w;

	const vec3 normal = normalize(cross(p2 - p0, p1 - p0));

	float foamMask = foamData(xyPos0);
	const float shininess = 1.0 - foamMask;

	foamMask *= texture2D(foam, 0.3 * p0.xz).y;
	
	const float sssFactor = clamp(SSS_POWER * (1.0 - normal.y), 0.0, 1.0)
			* max(-sunlightDir.y, 0.0);

	vec3 waterColor = mix(oceanColor0, oceanColor1, 0.2 * sssFactor);
	waterColor = mix(waterColor, vec3(1.0), foamMask);

	float metallic = 0.3;// 0.7 * (1.0 - sssFactor);
	metallic *= (1.0 - foamMask);

	float lightWeight = 1.0 - sssFactor;
	lightWeight = mix(fma(lightWeight, 0.4, 0.6), 1.0, foamMask);

	outColor = vec4(waterColor, 1.0);
	outNormal = vec4(normal, 1.0);
	outLighting = vec4(metallic, 0.1 + 0.9 * foamMask, 0.5 * foamMask + 0.5, lightWeight);
}

#endif
