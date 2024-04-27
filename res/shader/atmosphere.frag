#version 450

layout(set = 0, binding = 0) uniform Atmosphere {
	vec3 defractionCoef; 
	float time;
};

layout(set = 1, binding = 0) uniform sampler2D scene;
layout(set = 1, binding = 1) uniform sampler2D normSampler; // TODO: don't need this

// layout(input_attachment_index=1, binding=0) uniform subpassInput depth;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(inUV, 0.0f, 1.0f);
	// float dv = subpassLoad(depth).x;
	// outColor = vec4(dv, dv, dv, 1.0f);
	outColor = vec4(texture(scene, inUV).rgb, 1.0f);
}
