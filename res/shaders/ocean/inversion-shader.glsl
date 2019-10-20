
#if defined(CS_BUILD)

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform image2D displacement;

layout (binding = 1, rgba32f) readonly uniform image2D buffer0;
layout (binding = 2, rgba32f) readonly uniform image2D buffer1;

uniform int N;
uniform int bufferNum;
uniform vec3 mask; 

const float perms[] = {1.0, -1.0};

void main() {
	const ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	const float perm = perms[int(mod((int(x.x + x.y)), 2))];
	vec3 base = imageLoad(displacement, x).rgb;
	base -= base * mask;
	
	if (bufferNum == 0) {
		const float h = imageLoad(buffer0, x).r;
		imageStore(displacement, x, vec4(base + mask * (perm * h / float(N * N)), 1.0));
	}
	else {
		const float h = imageLoad(buffer1, x).r;
		imageStore(displacement, x, vec4(base + mask * (perm * h / float(N * N)), 1.0));
	}
}

#endif
