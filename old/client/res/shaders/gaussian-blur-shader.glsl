
#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform image2D buffer0;
layout (binding = 1, rgba32f) uniform image2D buffer1;

uniform bool horizontal;

const float weight[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);

	if (horizontal) {
		vec3 result = imageLoad(buffer0, x).rgb * weight[0];

		for (int i = 1; i < 5; ++i) {
			result += imageLoad(buffer0, x + ivec2(i, 0)).rgb * weight[i];
			result += imageLoad(buffer0, x - ivec2(i, 0)).rgb * weight[i];
		}

		imageStore(buffer1, x, vec4(0.5 * result, 1.0));
	}
	else {
		vec3 result = imageLoad(buffer1, x).rgb * weight[0];

		for (int i = 1; i < 5; ++i) {
			result += imageLoad(buffer1, x + ivec2(0, i)).rgb * weight[i];
			result += imageLoad(buffer1, x - ivec2(0, i)).rgb * weight[i];
		}

		imageStore(buffer0, x, vec4(result, 1.0));
	}
}

#endif

