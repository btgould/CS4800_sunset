#version 450

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
layout(location = 4) in vec3 camPos;

layout(location = 0) out vec4 outColor;

void main() {
	// Lambertian lighting
	vec3 norm = normalize(texture(normSampler, fragTexCoord).rgb);
    vec3 lightDir = normalize(light.pos - fragPos); 
    float diffuse = max(dot(norm, lightDir), 0.0) * light.diffuseStrength;
    vec3 lighting = (light.ambientStrength + diffuse) * light.color;

	// ugly hack to not light skybox 
	if (length(fragPos) > 500.0f) lighting = vec3(1.0f, 1.0, 1.0f); 

	// fog 
	float fogIntensity = length(camPos - fragPos) / 800; 
	fogIntensity = pow(fogIntensity, 5); // Near 0 until threshold, then grow quickly
	fogIntensity = min(fogIntensity, 0.8);
	vec3 fogColor = vec3(0.8f, 0.8f, 0.8f);

	// combine lighting w/ texture albedo + fog
	vec3 clearColor = lighting * texture(texSampler, fragTexCoord).rgb;
    outColor = vec4(mix(clearColor, fogColor, fogIntensity) , 1.0f);
}
