#version 330
in vec3 texcoord;

uniform samplerCube skybox;

layout(location = 0) out vec4 out_color;

void main()
{    
    out_color = texture(skybox, texcoord);
}