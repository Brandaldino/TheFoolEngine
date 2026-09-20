#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_LightViewProjection;
// uniform mat4 u_Model;

layout(std430, binding = 1) buffer InstanceBuffer
{
    mat4 u_InstanceModels[];
};

void main()
{
    mat4 model = u_InstanceModels[gl_InstanceID];
    gl_Position = u_LightViewProjection * model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{
    
}