#include "ocean-common.glh"

#define MAX_OBJECTS 128

#if defined(CS_BUILD)

layout (local_size_x = 1, local_size_y = 1) in;

layout (std430, binding = 0) buffer data {
	mat4 transforms[MAX_OBJECTS];
	vec2 scales[MAX_OBJECTS];
};

uniform sampler2D displacement;

float height(vec2 pos) {
	return texture2D(displacement, OCEAN_SAMPLE * pos).y
			* amplitude;
}

#define MIX_FACTOR 0.2

void main() {
	const int i = int(gl_GlobalInvocationID.x);

	vec3 pos = transforms[i][3].xyz / transforms[i][3][3];
	vec3 right = transforms[i][0].xyz;
	vec3 forward = transforms[i][2].xyz;

	vec3 ptFwd = pos + forward * scales[i].y;
	vec3 ptBack = pos - forward * scales[i].y;

	vec3 ptLeft = pos - right * scales[i].x;
	vec3 ptRight = pos + right * scales[i].x;

	ptFwd.y = height(ptFwd.xz);
	ptBack.y = height(ptBack.xz);
	
	ptLeft.y = height(ptLeft.xz);
	ptRight.y = height(ptRight.xz);

	right = mix(right, normalize(ptRight - ptLeft), MIX_FACTOR);
	forward = mix(forward, normalize(ptFwd - ptBack), MIX_FACTOR);
	vec3 up = cross(forward, right);

	pos.y = mix(pos.y, 0.25 * (ptFwd.y + ptBack.y + ptLeft.y + ptRight.y), MIX_FACTOR);
	//pos.y = height(pos.xz);

	transforms[i] = mat4(vec4(right, 0.0), vec4(up, 0.0),
			vec4(forward, 0.0), vec4(pos, 1.0));
}

#endif
