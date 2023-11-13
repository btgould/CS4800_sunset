#version 450

layout(push_constant) uniform TRS {
    mat4 trs;
} trs;

layout(set = 0, binding = 0) uniform VP {
    mat4 vp;
} camVP;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
	gl_Position = camVP.vp * trs.trs * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
