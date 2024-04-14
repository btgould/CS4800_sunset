#version 450
#extension GL_EXT_scalar_block_layout : require // pack alignment of integer array

layout(std430, set = 0, binding = 1) uniform GRID {
	uint cellCount; 
	uint cells[10][10]; // TODO: no clue how to get this value from cpp #define
} gridData;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normSampler; // TODO: don't need this

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const float sqrt3 = sqrt(3);
const float invSqrt3 = 1.0f / sqrt3;
const mat2 cart2hex = mat2(2 * invSqrt3, 0.0f, invSqrt3, 1.0f);
const mat2 hex2cart = mat2(sqrt3 / 2.0f, 0.0f, -0.5f, 1.0f);

// Gets the index of the hexagon the given point belongs to. 
// Hexagons are sqrt(3)/2 wide, and the grid is centered at the origin. 
vec2 hexagon(vec2 point) {
	// Convert from cartesian coords to hex axes 
	vec2 q = point * cart2hex; 

	vec2 qInt = floor(q); 
	vec2 qFrac = fract(q);
	float v = mod(qInt.x + qInt.y, 3.0f);

	float ca = step(1.0f, v);
	float cb = step(2.0f, v);
	vec2  ma = step(qFrac.xy, qFrac.yx);

	return (qInt + ca - cb * ma) * hex2cart;
}

void main() {
	vec2 hexTexCoord = fragTexCoord * (gridData.cellCount + 1); // long rows need to be 1 bigger than numCells
	hexTexCoord.x -= 0.5f;
	hexTexCoord.y -= invSqrt3;
	hexTexCoord *= sqrt3;

	vec2 hexCoord = hexagon(hexTexCoord) * invSqrt3;
	vec2 inside = step(0, hexCoord) - step(gridData.cellCount, hexCoord);
	hexCoord *= inside.x * inside.y;

	outColor = vec4(hexCoord / gridData.cellCount, 0.0f, 1.0f);

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
