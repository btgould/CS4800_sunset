#version 450

layout(set = 0, binding = 0) uniform VP {
    mat4 vp;
} camVP; 

layout(set = 0, binding = 1) uniform ATMOSPHERE {
	vec3 center;
	ivec3 wavelengths;
	vec3 defractionCoef; 
	float time;
	float radius;
	float offsetFactor;
	float densityFalloff;
	float scatteringStrength;
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

const float maxFloat = 3.402823466e+38;

vec2 raySphere(vec3 origin, vec3 dir, vec3 center, float radius) {
	vec3 offset = origin - center;

	float a = dot(dir, dir);
	float b = 2.0f * dot(dir, offset);
	float c = dot(offset, offset) - radius * radius;
	float d = b * b - 4 * a * c; // Discriminant from quadratic formula

		// Number of intersections: 0 when d < 0; 1 when d = 0; 2 when d > 0
		if (d > 0) {
			float s = sqrt(d);
			float dstToSphereNear = max(0, (-b - s) / (2.0 * a));
			float dstToSphereFar = (-b + s) / (2.0 * a);

			// Ignore intersections that occur behind the ray
			if (dstToSphereFar >= 0) {
				return vec2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
			}
		}

		// Ray did not intersect sphere
		return vec2(maxFloat, 0);
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

vec3 calculateLight(vec3 eyePos, vec3 dir, float atmosLen) {
	vec3 inScatterPoint = eyePos; 
	float stepSize = atmosLen / (atmos.numInScatteringPoints - 1.0f); 
	vec3 inScatteredLight = vec3(0.0f);
	
	for (int i = 0; i < atmos.numInScatteringPoints; i++) {
		vec3 dirToSun = normalize(light.pos - inScatterPoint);
		float sunRayLen = raySphere(inScatterPoint, dirToSun, atmos.center, atmos.radius).y;
		float sunRayOpticalDepth = opticalDepth(inScatterPoint, dirToSun, sunRayLen); // Rayleigh in scattering 
		float viewRayOpticalDepth = opticalDepth(inScatterPoint, -dir, stepSize * i); // Rayleigh out scattering
		vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * atmos.defractionCoef);
		float localDensity = densityAtPoint(inScatterPoint);

		inScatteredLight += localDensity * transmittance * atmos.defractionCoef * stepSize; 
		inScatterPoint += dir * stepSize;
	}

	return inScatteredLight;
}

void main() {
	vec4 clipPos = vec4(inUV * 2.0 - 1.0, 1.0, 1.0);
	vec4 viewPos = inverse(camVP.vp) * clipPos;
	vec3 pos = vec3(0.0f, 0.0f, 0.0f);
	vec3 dir = (viewPos / viewPos.w).xyz; // HACK: assuming camera is at origin
	dir = normalize(dir);

	float planetRadius = atmos.radius * atmos.offsetFactor - 0.1f;

	vec2 planetDist = raySphere(pos, dir, atmos.center, planetRadius);
	vec2 atmosDist = raySphere(pos, dir, atmos.center, atmos.radius);
	float distInAtmos = min(atmosDist.y, planetDist.x - atmosDist.x);

	if (atmosDist.y < planetDist.x) {
		float intensity = distInAtmos; // / (2.0f * atmos.radius);
		intensity = planetDist.x - atmosDist.x;
		vec3 light = calculateLight(vec3(pos), dir, distInAtmos);

		outColor = vec4(light, 1.0f);
	} else {
		outColor = vec4(0.80f, 0.43f, 0.18f, 1.0f);
	}
}
