#version 330

in vec2 vPos;
in vec2 vUV;
in vec3 vColor;
in float vTextureIndex;

out vec2 fPos;
out vec2 fUV;
out vec3 fColor;
out float fTextureIndex;

void main(){
	gl_Position = vec4(vPos, 0.0, 1.0);
	fPos = vPos;
	fUV = vUV;
	fColor = vColor;
	fTextureIndex = vTextureIndex;
}

