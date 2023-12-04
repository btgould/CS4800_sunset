#version 450

layout(set = 0, binding = 1) uniform CLOUD {
    vec3 pos;
	vec3 scale;
} cloud;

layout(set = 0, binding = 2) uniform LIGHT {
	vec3 pos; 
	vec3 color; 
	float ambientStrength; 
	float diffuseStrength;
} light;
layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	// Lambertian lighting
	vec3 norm = normalize(texture(normSampler, fragTexCoord).rgb);
    vec3 lightDir = normalize(light.pos - fragPos); 
    float diffuse = max(dot(norm, lightDir), 0.0) * light.diffuseStrength;
    vec3 lighting = (light.ambientStrength + diffuse) * light.color;

	// ugly hack to not light skybox 
	if (length(fragPos) > 500.0f) lighting = vec3(1.0f, 1.0, 1.0f); 

	// combine lighting w/ texture albedo
	outColor = vec4(lighting * texture(texSampler, fragTexCoord).rgb, 1.0f);

	// ugly hack to extract cloud 
	vec3 fragDist = (fragPos - cloud.pos) / cloud.scale;
	if (length(fragDist) < 1) {
		float normalizedHeight = (fragPos.z - cloud.pos.z) / cloud.scale.z;
		normalizedHeight = (normalizedHeight + 1) / 2;
		float intensity = pow(normalizedHeight, 0.5); // TODO: This can be faster
		vec3 baseColor = intensity * vec3(0.9f, 0.9f, 0.9f);
		outColor = vec4(baseColor, 0.8f);
	}
}
