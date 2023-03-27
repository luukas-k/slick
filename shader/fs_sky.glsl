#version 330

in vec3 fPos;

uniform samplerCube tex_skybox;

out vec4 rColor;

void main(){
    rColor = vec4(texture(tex_skybox, fPos).xyz, 1.0);
}
