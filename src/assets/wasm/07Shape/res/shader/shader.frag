#version 300 es
precision mediump float;
precision mediump int;
precision mediump sampler2DArray;

in vec4 fP;
in vec4 fC;
in vec4 fL;
in vec3 fN;

out vec4 color;

void main(void)
{       
    vec3 o =-normalize(fP.xyz);
    vec3 n = normalize(fN);
    vec3 r = reflect(o,n);
    vec3 l = normalize(fL.xyz-fP.xyz);

    float a = 0.1;
    float d = 0.7*max(0.0,dot(n,l));
    float s = 0.6*pow(max(0.0,-dot(r,l)),4.0);

    color = fC * ( a + d + s);
}