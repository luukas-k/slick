#version 330

in vec2 fUV;

uniform sampler2D sys_texture;

out vec4 rColor;

void main(){
	float a = texture(sys_texture, fUV).x;
	rColor = vec4(vec3(0), a);
}

