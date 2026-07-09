#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_Direction;

void main()
{
    v_Direction = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

in vec3 v_Direction;

uniform sampler2D u_EquirectMap;

out vec4 FragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);

void main()
{
    vec3 dir = normalize(v_Direction);
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
    uv = uv * invAtan + 0.5;
    vec3 color = texture(u_EquirectMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}