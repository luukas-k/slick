#version 330

in vec3 fPos;
in vec3 fColor;

layout(location = 0) out vec4 rColor;
layout(location = 1) out uint rId;

void main(){
    rColor = vec4(fColor, 1.0);
    rId = uint(5);
}
