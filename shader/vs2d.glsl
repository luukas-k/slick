#version 330

in vec2 vPos;
in vec2 vUV;

out vec2 fUV;

void main(){
	gl_Position = vec4(vPos, 0.0, 1.0);
	fUV = vUV;
}

