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

bool inHex(vec2 point, vec2 pos, float size) {
	float invSqrt3 = 0.57735026919;
	float margin = (1 - invSqrt3) / 2;
	point = point - pos; 
	point = point / size;

	float slope = 2 * margin * point.x;

	return 0 <= point.x && point.x <= 1 && 
		-slope + margin <= point.y &&
		slope - margin <= point.y &&
		point.y <= slope + 1 - margin && 
		point.y <= -slope + 1 + margin;
}

void main() {
	// map pixels to grid cells
	vec2 cell = floor(gridData.cellCount * fragTexCoord);
	ivec2 icell = ivec2(cell);

	outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f); // if unknown
	outColor = float(inHex(fragTexCoord, vec2(0.5f, 0.0f), 0.5f)) * vec4(1.0f, 0.0f, 0.0f, 1.0f);

	// get color for type of grid cell
	/* uint cellType = gridData.cells[icell.x][icell.y];
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
	} */
}
