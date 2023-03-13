#version 330

in vec2 vPos;
in vec2 vUV;
in vec3 vColor;

out vec2 fPos;
out vec2 fUV;
out vec3 fColor;

void main(){
	gl_Position = vec4(vPos, 0.0, 1.0);
	fPos = vPos;
	fUV = vUV;
	fColor = vColor;
}

