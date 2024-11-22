in vec3 position;
in vec3 normal;
in vec4 color;

uniform mat4 p;
uniform mat4 v;

out vec4 fP;
out vec4 fC;
out vec4 fL;
out vec3 fN;

void main(void)
{
    fP = v * vec4(position,1.0);
    fL = v * vec4(0.0,0.0,1.0,1.0);

    fC = color;
    fN= vec3(v * vec4(normal,0.0));

    gl_Position = p * fP;
}
