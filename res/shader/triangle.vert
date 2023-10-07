#version 450

layout(binding = 0) uniform TRS {
    mat4 trs;
} trs;

layout(binding = 1) uniform MVP {
    mat4 vp;
} mvp;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
	gl_Position = mvp.vp * trs.trs * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
