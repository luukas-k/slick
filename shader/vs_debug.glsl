#version 330

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;

uniform mat4 sys_proj = mat4(1);
uniform mat4 sys_view = mat4(1);

out vec3 fPos;
out vec3 fColor;

void main(){
	gl_Position = sys_proj * sys_view * vec4(vPos, 1.0);
	fPos = normalize(vPos);
	fColor = vColor;
}
