#version 330

in vec2 fPos;
in vec2 fUV;
in vec3 fColor;
in float fTextureIndex;

uniform sampler2D sys_textures[16];

out vec4 rColor;

vec4 get_color(){
	if(fTextureIndex >= -0.5){
		return texture(sys_textures[int(fTextureIndex)], fUV);
	}
	else{
		return vec4(fColor, 1.0);
	}
}

void main(){
	rColor = get_color();
}