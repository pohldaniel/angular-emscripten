#version 300 es
precision mediump float;
precision mediump int;

in vec2 texCoord;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;

out vec4 colorOut;

void main(void){
	colorOut = vec4 (normal, 1.0);
}