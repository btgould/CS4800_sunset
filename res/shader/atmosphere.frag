#version 450

layout(set = 0, binding = 0) uniform VP {
    mat4 vp;
} camVP; 

layout(set = 0, binding = 1) uniform ATMOSPHERE {
	vec3 defractionCoef; 
	float time;
	float radius;
	float offsetFactor;
} atmos;

layout(set = 1, binding = 0) uniform sampler2D scene;
layout(set = 1, binding = 1) uniform sampler2D normSampler; // TODO: don't need this

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

float distInsideSphere(vec3 origin, vec3 dir, vec3 center, float radius) {
	vec3 offset = origin - center;

	float a = dot(dir, dir);
	float b = 2.0f * dot(dir, offset);
	float c = dot(offset, offset) - radius * radius;

	return (-b + sqrt(b * b - 4.0f * a * c)) / (2.0f * a);
}



void main() {
	vec4 clipPos = vec4(inUV * 2.0 - 1.0, 1.0, 1.0);
	vec4 viewPos = inverse(camVP.vp) * clipPos;
	vec3 dir = (viewPos / viewPos.w).xyz; // HACK: assuming camera is at origin
	dir = normalize(dir);

	vec3 atmosCenter = -vec3(0.0f, atmos.radius, 0.0f) * atmos.offsetFactor;

	float atmosDist = distInsideSphere(vec3(0.0f, 0.0f, 0.0f), dir, atmosCenter, atmos.radius);
	float intensity = atmosDist / atmos.radius;

	outColor = vec4(0.0f, 0.0f, intensity, 1.0f);
}
