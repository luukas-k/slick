#version 330

in vec2 fPos;
in vec2 fUV;
in vec3 fColor;
in float fTextureIndex;
in vec2 fQuadSize;
in float fBorderRadius;
in float fIsText;

uniform sampler2D sys_textures[16];
uniform vec2 sys_viewport = vec2(1,1);

out vec4 rColor;

vec4 get_color(){
	if(fIsText < 0.5f){
		if(fTextureIndex >= -0.5){
			return texture(sys_textures[int(fTextureIndex)], fUV);
		}
		else{
			return vec4(fColor, 1.0);
		}
	} 
	else {
		float alpha = texture(sys_textures[int(fTextureIndex)], fUV).r;
		return vec4(vec3(0.0), alpha);
	}
}

void main(){
	// const float b_radius = 0.05;
	float b_radius = fBorderRadius; //fBorderRadius;


	vec2 coords = fUV * fQuadSize;

	if(fIsText < 0.5){
		if (
			coords.x < b_radius && coords.y < b_radius && distance(coords, vec2(b_radius, b_radius)) > b_radius ||
			fQuadSize.x - coords.x < b_radius && coords.y < b_radius && distance(coords, vec2(fQuadSize.x - b_radius, b_radius)) > b_radius ||
			coords.x < b_radius && fQuadSize.y - coords.y < b_radius && distance(coords, vec2(b_radius, fQuadSize.y - b_radius)) > b_radius ||
			fQuadSize.x - coords.x < b_radius && fQuadSize.y - coords.y < b_radius && distance(coords, vec2(fQuadSize.x - b_radius, fQuadSize.y - b_radius)) > b_radius 
		) {
			discard;
		}
	}
	
	rColor = get_color();
}