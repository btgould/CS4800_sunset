#version 450

layout (location = 0) out vec2 outUV;

// Pass constructed position to rest of shader stages 
out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	// Construct position from vertex index 
	// 0 -> (0, 0)
	// 1 -> (2, 0)
	// 2 -> (0, 2) 
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}

