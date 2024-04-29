#version 450

layout(set = 0, binding = 0) uniform VP {
    mat4 vp;
} camVP; 

layout(set = 0, binding = 1) uniform ATMOSPHERE {
	vec3 center;
	vec3 defractionCoef; 
	float time;
	float radius;
	float offsetFactor;
	float densityFalloff;
	int numInScatteringPoints;
	int numOpticalDepthPoints;
} atmos;

layout(set = 0, binding = 2) uniform LIGHT {
	vec3 pos; 
	vec3 color; 
	float ambientStrength; 
	float diffuseStrength;
} light;

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

float densityAtPoint(vec3 samplePoint) {
	float heightAboveSuface = length(samplePoint - atmos.center) - (atmos.radius * atmos.offsetFactor);
	float heightNormalized = heightAboveSuface / (atmos.radius * (1 - atmos.offsetFactor));
	float localDensity = exp(-heightNormalized * atmos.densityFalloff) * (1 - heightNormalized);
	return localDensity;
}

float opticalDepth(vec3 origin, vec3 dir, float len) {
	vec3 densitySamplePoint = origin; 
	float stepSize = len / (atmos.numOpticalDepthPoints - 1); 
	float opticalDepth = 0.0f;

	for (int i = 0; i < atmos.numOpticalDepthPoints; i++) {
		float localDensity = densityAtPoint(densitySamplePoint);
		opticalDepth += localDensity * stepSize; 
		densitySamplePoint += dir * stepSize;
	}

	return opticalDepth;
}

float calculateLight(vec3 eyePos, vec3 dir, float atmosLen) {
	vec3 inScatterPoint = eyePos; 
	float stepSize = atmosLen / (atmos.numInScatteringPoints - 1.0f); 
	float inScatteredLight = 0.0f;
	
	for (int i = 0; i < atmos.numInScatteringPoints; i++) {
		vec3 dirToSun = light.pos - inScatterPoint;
		float sunRayLen = distInsideSphere(inScatterPoint, dirToSun, atmos.center, atmos.radius);
		float sunRayOpticalDepth = opticalDepth(inScatterPoint, dirToSun, sunRayLen); // Raleigh in scattering 
		float viewRayOpticalDepth = opticalDepth(inScatterPoint, -dir, stepSize * i); // Raleigh out scattering
		float transmittance = exp(-sunRayOpticalDepth - viewRayOpticalDepth);
		float localDensity = densityAtPoint(inScatterPoint);

		inScatteredLight += localDensity * transmittance * stepSize; 
		inScatterPoint += dir * stepSize;
	}

	return inScatteredLight;

}

void main() {
	vec4 clipPos = vec4(inUV * 2.0 - 1.0, 1.0, 1.0);
	vec4 viewPos = inverse(camVP.vp) * clipPos;
	vec3 dir = (viewPos / viewPos.w).xyz; // HACK: assuming camera is at origin
	dir = normalize(dir);

	float atmosDist = distInsideSphere(vec3(0.0f, 0.0f, 0.0f), dir, atmos.center, atmos.radius);
	// float intensity = atmosDist / atmos.radius;
	float light = calculateLight(vec3(0.0f, 0.0f, 0.0f), dir, atmosDist) / atmos.radius;

	// float intensity = opticalDepth(vec3(0.0f, 0.0f, 0.0f), dir, atmosDist) / atmos.radius;

	outColor = vec4(0.0f, 0.0f, light, 1.0f);
}
