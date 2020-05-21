
#if defined(VS_BUILD)

layout (location = 0) in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}

#elif defined(FS_BUILD)

uniform sampler2D scene;
uniform sampler2D brightBlur;

layout (location = 0) out vec4 outColor;

void main() {
	const vec3 sceneColor = texelFetch(scene, ivec2(gl_FragCoord.xy), 0).rgb;
	const vec3 brightColor = texelFetch(brightBlur, ivec2(gl_FragCoord.xy), 0).rgb;

	outColor = vec4(sceneColor + brightColor, 1.0);
}

#endif
