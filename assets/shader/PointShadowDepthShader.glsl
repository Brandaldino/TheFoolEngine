#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_LightViewProjection;
uniform mat4 u_Model;

out vec3 v_FragPos;

void main()
{
    v_FragPos = (u_Model * vec4(a_Position, 1.0)).xyz;
    gl_Position = u_LightViewProjection * vec4(v_FragPos, 1.0);
}


#type fragment
#version 450 core

in vec3 v_FragPos;

uniform vec3 u_LightPos;
uniform float u_FarPlane;

layout(location = 0) out vec4 color;

void main()
{
    float dist = length(v_FragPos - u_LightPos);
    color = vec4(dist / u_FarPlane, 0.0, 0.0, 1.0); // Linear distance normalization
    // color = vec4(0.5, 0.5, 0.5, 1.0);
}