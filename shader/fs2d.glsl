#version 330

in vec2 fUV;

uniform vec3 sys_color;

out vec4 rColor;

void main(){
	rColor = vec4(sys_color, 1.0);
}