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
uniform float u_Threshold;

out vec4 FragColor;

void main()
{
    vec3 color = texture(u_HDRColor, v_TexCoord).rgb;

    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float amount = clamp(luminance - u_Threshold, 0.0, 1.0);

    FragColor = vec4(color * amount, 1.0);
}