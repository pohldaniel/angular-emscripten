#version 300 es
precision mediump float;
precision mediump int;
precision mediump sampler2DArray;

uniform samplerCube envMap;
uniform float u_alpha;
uniform vec3 u_color;

in vec4 P; // position
in vec3 N; // normal
in vec3 I; // incident vector

out vec4 color;

float my_fresnel(vec3 I, vec3 N, float power, float scale, float bias) {
    return bias + (pow(clamp(1.0 - dot(I, N), 0.0, 1.0), power) * scale);
}

void main(){

	vec3 I = normalize(I);
    vec3 N = normalize(N);

    vec3 R = reflect(I, N);
    vec3 T = refract(I, N, 0.95);
    float fresnel = my_fresnel(-I, N, 5.0, 0.99, 0.01);
	
	vec3 Creflect = texture(envMap, R).rgb;
	vec3 Crefract = texture(envMap, T).rgb;
	Crefract *= u_color;

	vec3 Cout = mix(Crefract, Creflect, 0.02);

    color = vec4(Cout, u_alpha);
}