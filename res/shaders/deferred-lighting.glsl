#include "common.glh"

#include "scene-info.glh"
#include "lighting.glh"
#include "normal-encoding.glh"

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

#define MAX_REFLECTION_LOD 4.0
#define MIN_SUNLIGHT_COS 0.2
#define SUN_RADIANCE 2.0

float distributionGGX(vec3 N, vec3 H, float roughness) {
	float a2 = roughness * roughness;
	a2 *= a2;

	float nDotH2 = max(dot(N, H), 0.0);
	nDotH2 *= nDotH2;

	float denom = nDotH2 * (a2 - 1.0) + 1.0;
	denom = M_PI * denom * denom;

	//return a2 / max(nDotH2 * (a2 - 1.0) + 1.0, 0.001);
	return a2 / denom;
}

float geometrySchlickGGX(float nDotV, float roughness) {
	float k = roughness + 1.0;
	k = (k * k) / 8.0;

	return nDotV / (nDotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	const float ggx1 = geometrySchlickGGX(max(dot(N, V), 0.0), roughness);
	const float ggx2 = geometrySchlickGGX(max(dot(N, L), 0.0), roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	const float ct = 1.0 - cosTheta;

	float ct5 = ct * ct;
	ct5 = ct5 * ct5 * ct;

	return F0 + (vec3(1.0) - F0) * ct5;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

uniform sampler2D colorBuffer; // vec3 color
uniform sampler2D normalBuffer; // vec3 normal
uniform sampler2D lightingBuffer; // float metallicity, float roughness, float ao, float lightPower

uniform sampler2D depthBuffer; // float depth

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

layout (location = 0) out vec4 outColor;
layout (location = 3) out vec4 brightColor;

void main() {
	const vec2 screenPosition = fma(gl_FragCoord.xy / displaySize, vec2(2.0), vec2(-1.0));
	const ivec2 texel = ivec2(gl_FragCoord.xy);

	const vec4 colorSpec = texelFetch(colorBuffer, texel, 0);
	const vec4 normal_ = texelFetch(normalBuffer, texel, 0);
	const vec4 lighting = texelFetch(lightingBuffer, texel, 0);
	const float depth = fma(texelFetch(depthBuffer, texel, 0).x, 2.0, -1.0);

	//const vec3 albedo = pow(colorSpec.xyz, vec3(2.2));
	const vec3 albedo = colorSpec.xyz;

	const vec3 normal = normal_.xyz;
	//const vec3 normal = decodeNormal(normal_);

	const vec4 rawPosition = invVP * vec4(screenPosition, depth, 1.0);
	const vec3 position = rawPosition.xyz / rawPosition.w;

	vec3 pointToEye = cameraPosition - position;
	const float cameraDist = length(pointToEye);
	pointToEye /= cameraDist;

	const float metallic = lighting.x;
	const float roughness = lighting.y;
	const float ao = lighting.z;
	const float lightWeight = lighting.w;

	const vec3 F0 = mix(vec3(0.04), albedo, metallic);

	vec3 Lo = vec3(0.0);

	// BEGIN SUNLIGHT VALUE CALCULATIONS
	const vec3 L = -sunlightDir;
	const vec3 H = normalize(pointToEye + L);
	const vec3 radiance = vec3(SUN_RADIANCE);

	const float NDF = distributionGGX(normal, H, roughness);
	const float G = geometrySmith(normal, pointToEye, L, roughness);
	vec3 F = fresnelSchlick(clamp(dot(H, pointToEye), 0.0, 1.0), F0);

	const float specDenom = 4.0 * max(dot(normal, pointToEye), 0.0)
			* max(dot(normal, L), 0.0);
	vec3 specular = (NDF * G * F) / max(specDenom, 0.001);
	
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallic;

	Lo += (kD * albedo / M_PI + specular)
			* radiance * max(dot(normal, L), MIN_SUNLIGHT_COS);
	// END SUNLIGHT VALUE CALCULATIONS

	// BEGIN IBL IRRADIANCE CALCULATIONS
	F = fresnelSchlickRoughness(max(dot(normal, pointToEye), 0.0), F0, roughness);
	kD = vec3(1.0) - F;
	kD *= 1.0 - metallic;

	vec3 ambient = kD * texture(irradianceMap, normal).rgb * albedo;
	// END IBL IRRADIANCE CALCULATIONS

	// BEGIN SPECULAR IBL CALCULATIONS
	const vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, pointToEye), 0.0), roughness)).rg;
	specular = textureLod(prefilterMap, reflect(-pointToEye, normal), roughness * MAX_REFLECTION_LOD).rgb
			* fma(F, vec3(brdf.x), vec3(brdf.y));
	
	ambient += specular;
	// END SPECULAR IBL CALCULATIONS

	vec3 inColor = ambient * ao + Lo;

	inColor = mix(albedo, inColor, lightWeight);
	
	const float fogVisibility = clamp(exp(-pow(cameraDist * fogDensity, fogGradient)), 0.0, 1.0);
	inColor = mix(fogColor, inColor, fogVisibility);

	const float brightness = dot(inColor, BRIGHT_THRESH);

	outColor = vec4(inColor, 1.0);

	if (brightness > 1.0) {
		brightColor = vec4(inColor, 1.0);
	}
	else {
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

#endif

