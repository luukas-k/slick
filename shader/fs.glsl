#version 330

in vec3 fPosition;
in vec3 fNormal;
in vec3 fTangent;
in vec2 fUV;
in mat3 fTBN;

uniform vec3 cam_pos = vec3(0);

// MATERIAL DATA

// Albedo
uniform int mat_has_base_color_texture = 0;
uniform vec3 mat_base_color = vec3(0.5);
uniform sampler2D mat_base_color_texture;

vec3 get_base_color(){
	if(mat_has_base_color_texture == 1){
		return texture(mat_base_color_texture, fUV).rgb;
	}
	return mat_base_color;
}

// Metallic & Roughness
uniform int mat_has_metallic_roughness = 0;
uniform float mat_metallic1 = 0.5;
uniform float mat_roughness1 = 0.5;
uniform sampler2D mat_metallic_roughness_texture;

float get_metallic(){
    if(mat_has_metallic_roughness == 1){
        return texture(mat_metallic_roughness_texture, fUV).r;
    }
    return mat_metallic1;
}
float get_roughness(){
    if(mat_has_metallic_roughness == 1){
        return texture(mat_metallic_roughness_texture, fUV).g;
    }
    return mat_roughness1;
}

// Normal map
uniform int mat_has_normal = 0;
uniform sampler2D mat_normal_map;

vec3 get_normal(){
    if(mat_has_normal == 1){
        return normalize(fTBN * (texture(mat_normal_map, fUV).xyz * 2.0 - 1.0));
    }
    return fNormal;
}

float get_alpha(){
    return texture(mat_base_color_texture, fUV).a;
}

// Lights
uniform vec3 light_color = vec3(1.0, 1.0, 1.0);
uniform vec3 light_pos = vec3(5, 5, 5);

out vec4 rColor;

const float PI = 3.14159;

float distGGX(float ndoth, float roughness){
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.00001);
}

float geomSmith(float ndotv, float ndotl, float roughness){
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = ndotv / (ndotv * (1.0 - k) + k);
    float ggx2 = ndotl / (ndotl * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnelShlick(float hdotv, vec3 baseReflectivity){
    return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - hdotv, 5.0);
}

void main(){
    vec3 norm = get_normal();

	vec3 N = normalize(norm);
    vec3 V = normalize(cam_pos - fPosition);

    vec3 baseRefl = mix(vec3(0.04), get_base_color(), get_metallic());

    vec3 Lo = vec3(0.0);

	for(int i = 0; i < 1; i++){
        vec3 L = normalize(light_pos - fPosition);
        vec3 H = normalize(V + L);
        float dist = length(light_pos - fPosition);
        float atten = 1.0 / (dist * dist);
        atten = 1.0;
        vec3 radiance = light_color * atten;

        float ndotv = max(dot(N, V), 0.00001);
        float ndotl = max(dot(N, L), 0.00001);
        float hdotv = max(dot(H, V), 0.0);
        float ndoth = max(dot(N, H), 0.0);

        float D = distGGX(ndoth, get_roughness());
        float G = geomSmith(ndotv, ndotl, get_roughness());
        vec3 F = fresnelShlick(hdotv, baseRefl);

        vec3 spec = (D * G * F) / (4.0 * ndotv * ndotl);

        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 - get_metallic();

        Lo += (kD * get_base_color() / PI + spec) * radiance * ndotl;
    }

    vec3 ambient = vec3(0.03) * get_base_color();
	
    vec3 color = ambient + Lo;

    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.0 / 2.2));
    if(get_alpha() < 0.1)
        discard;

	rColor = vec4(color, 1.0);
}

