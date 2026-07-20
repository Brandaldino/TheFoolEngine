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
uniform float u_Exposure;

out vec4 FragColor;

void main()
{
    vec3 hdrColor = texture(u_HDRColor, v_TexCoord).rgb;

    // Exposure
    vec3 mapped = vec3(1.0) - exp(-hdrColor * u_Exposure);

    // Gamma Correction
    mapped = pow(mapped, vec3( 1.0 / 2.2 ));

    FragColor = vec4(mapped, 1.0);

    // Test
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}