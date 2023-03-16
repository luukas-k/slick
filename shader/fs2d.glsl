#version 330

in vec2 fPos;
in vec2 fUV;
in vec3 fColor;
in float fTextureIndex;
in float fQuadAspectRatio;
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
	float b_radius = fBorderRadius;

	if(fUV.x < b_radius / fQuadAspectRatio && fUV.y < b_radius && distance(vec2(b_radius), fUV * vec2(fQuadAspectRatio, 1.0)) > b_radius)
		discard;
	if(fUV.x > (1.0 - b_radius / fQuadAspectRatio) && fUV.y < b_radius && distance(vec2(b_radius, b_radius), (vec2(1.0 - fUV.x, fUV.y)) * vec2(fQuadAspectRatio, 1.0)) > b_radius)
		discard;
	if(fUV.x < b_radius / fQuadAspectRatio && fUV.y > (1.0 - b_radius) && distance(vec2(b_radius), vec2(fUV.x, 1.0 - fUV.y) * vec2(fQuadAspectRatio, 1.0)) > b_radius)
		discard;
	if(fUV.x > (1.0 - b_radius / fQuadAspectRatio) && fUV.y > (1.0 - b_radius) && distance(vec2(b_radius, b_radius), (vec2(1.0 - fUV.x, 1.0 - fUV.y)) * vec2(fQuadAspectRatio, 1.0)) > b_radius)
		discard;
	
	rColor = get_color();
}