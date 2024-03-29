#version 450
#extension GL_EXT_scalar_block_layout : require // pack alignment of integer array

layout(std430, set = 0, binding = 1) uniform GRID {
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
	// map pixels to grid cells
	vec2 cell = floor(gridData.cellCount * fragTexCoord);
	ivec2 icell = ivec2(cell);

	outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f); // if unknown

	// get color for type of grid cell
	uint cellType = gridData.cells[icell.x][icell.y];
	if (cellType == 1) {
		outColor = vec4(0.3f, 0.3f, 0.3f, 1.0f); // empty
	} else if (cellType == 2) {
		outColor = vec4(0.29f, 0.21f, 0.13f, 1.0f); // solid
	} else if (cellType == 4 || cellType == 8) {
		outColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); // water
	} else if (cellType == 16) {
		outColor = vec4(1.0f, 1.0f, 0.0f, 1.0f); // sand
	} else if (cellType == 32) {
		outColor = vec4(1.0f, 0.0f, 1.0f, 1.0f); // fungi
	}
}
