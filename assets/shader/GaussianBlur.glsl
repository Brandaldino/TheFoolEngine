#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_Position.xy * 0.5 + 0.5;
    gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450 core

in vec2 v_TexCoord;

uniform sampler2D u_Input;
uniform vec2 u_Direction; // (1, 0) = horizon, (0, 1) = vertical

out vec4 FragColor;

void main()
{
    vec2 texelSize = 1.0 / textureSize(u_Input, 0);
    vec3 result = texture(u_Input, v_TexCoord).rgb * 0.227;

    vec2 offset = u_Direction * texelSize;
    for(int i = 1; i < 5; ++i)
    {
        float weight = 0.076;
        result += texture(u_Input, v_TexCoord + offset * i).rgb * weight;
        result += texture(u_Input, v_TexCoord - offset * i).rgb * weight;
    }

    FragColor = vec4(result, 1.0);
}