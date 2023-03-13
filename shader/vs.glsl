#version 330

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec2 vUV;

uniform mat4 sys_proj = mat4(1);
uniform mat4 sys_view = mat4(1);
uniform mat4 sys_model = mat4(1);

out vec3 fPosition;
out vec3 fNormal;
out vec3 fTangent;
out vec2 fUV;
out mat3 fTBN;

void main(){
	gl_Position = sys_proj * sys_view * sys_model * vec4(vPos, 1.0);
	fPosition = (sys_model * vec4(vPos, 1.0)).xyz;
	fNormal = mat3(transpose(inverse(sys_model))) * vNormal;
	fTangent = mat3(transpose(inverse(sys_model))) * vTangent;
	fUV = vUV;

	vec3 T = normalize(vec3(sys_model * vec4(vTangent,   0.0)));
	vec3 B = normalize(vec3(sys_model * vec4(cross(vTangent, vNormal), 0.0)));
	vec3 N = normalize(vec3(sys_model * vec4(vNormal,    0.0)));
	fTBN = mat3(T, B, N);
}
