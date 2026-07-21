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

uniform sampler2D u_HDRColor;
uniform sampler2D u_BloomBlur;
uniform float u_Intensity;

out vec4 FragColor;

void main()
{
    vec3 hdr = texture(u_HDRColor, v_TexCoord).rgb;
    vec3 bloom = texture(u_BloomBlur, v_TexCoord).rgb;

    vec3 result = hdr + bloom * u_Intensity;

    FragColor = vec4(result, 1.0);
}