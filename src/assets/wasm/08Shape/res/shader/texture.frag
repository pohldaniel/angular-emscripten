#version 300 es
precision mediump float;
precision mediump int;
precision mediump sampler2DArray;

uniform sampler2D u_texture;

in vec2 texCoord;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;

out vec4 colorOut;

void main(void){
	colorOut = texture( u_texture, texCoord );
}