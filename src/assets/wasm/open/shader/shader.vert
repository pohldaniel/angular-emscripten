in vec3 position;
in vec3 normal;
in vec4 color;

uniform mat4 projection;
uniform mat4 view;

out vec4 fP;
out vec4 fC;
out vec4 fL;
out vec3 fN;

void main(void)
{
    fP = view * vec4(position,1.0);
    fL = view * vec4(0.0,0.0,1.0,1.0);

    fC = color;
    fN= vec3(view * vec4(normal,0.0));

    gl_Position = projection * fP;
}
