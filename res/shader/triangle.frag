#version 450

layout(set = 0, binding = 1) uniform LIGHT {
	vec3 pos; 
	vec3 color; 
	float ambientStrength; 
	float diffuseStrength;
} light;
layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	// Lambertian lighting
	vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light.pos - fragPos); 
    float diffuse = max(dot(norm, lightDir), 0.0) * light.diffuseStrength;
    vec3 lighting = (light.ambientStrength + diffuse) * light.color;

	// combine lighting w/ texture albedo
    outColor = vec4(lighting * texture(texSampler, fragTexCoord).rgb, 1.0f);
}
