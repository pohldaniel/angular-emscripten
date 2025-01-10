#version 300 es
precision mediump float;
precision mediump int;
precision mediump sampler2DArray;

uniform sampler2D u_texture;

in vec2 texCoord;
in vec3 normal;
in vec4 vertColor;

out vec4 color;

void main(void){
	color = vertColor;	
}