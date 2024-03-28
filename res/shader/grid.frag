#version 450

layout(set = 0, binding = 1) uniform GRID {
	uint cellCount; 
	uint cells[50][50]; // TODO: no clue how to get this value from cpp #define
} gridData;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normSampler; // TODO: don't need this

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec2 cell = floor(gridData.cellCount * fragTexCoord);
	float tmp = cell.x;
	cell.x = gridData.cellCount - cell.y;
	cell.y = tmp;

	outColor = vec4(cell / gridData.cellCount, 0.0f, 1.0f);
}
