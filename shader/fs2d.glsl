#version 330

in vec2 fPos;
in vec2 fUV;
in vec3 fColor;

out vec4 rColor;

void main(){
	rColor = vec4(fColor, 1.0);
}