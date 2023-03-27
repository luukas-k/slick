#version 330

layout(location = 0) in vec3 vPos;

uniform mat4 sys_proj = mat4(1);
uniform mat4 sys_view = mat4(1);

out vec3 fPos;

void main(){
	gl_Position = sys_proj * mat4(mat3(sys_view)) * vec4(vPos, 1.0);
	fPos = normalize(vPos);
}
