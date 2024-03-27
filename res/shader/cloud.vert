#version 450

layout(push_constant) uniform TRS {
    mat4 trs;
} trs;

layout(set = 0, binding = 0) uniform VP {
    mat4 vp;
} camVP;

layout(set = 0, binding = 1) uniform CloudSettings {
	float noiseFreq;
	float baseIntensity; 
	float opacity;
} cloud;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 cloudPos;
layout(location = 2) out vec3 cloudScale;

// Generic noise from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83 
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p) {
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

void main() {
	fragPos = vec3(trs.trs * vec4(inPosition, 1.0));
	gl_Position = camVP.vp * vec4(fragPos, 1.0);
	gl_Position = gl_Position + 30 * noise(fragPos / cloud.noiseFreq); // random cloud geometry

	cloudPos = vec3(trs.trs[3]);
	cloudScale = vec3(0);
	cloudScale.x = sign(trs.trs[0][0]) * length(vec3(trs.trs[0]));
	cloudScale.y = sign(trs.trs[1][1]) * length(vec3(trs.trs[1]));
	cloudScale.z = sign(trs.trs[2][2]) * length(vec3(trs.trs[2]));
}
