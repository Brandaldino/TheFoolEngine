#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_Direction;

void main()
{
    v_Direction = a_Position;

    // Remove the translation from the view matrix (camera always at origin) to ensure the skybox doesn't follow the camera.
    mat4 viewNoTranslate = mat4(mat3(u_View));
    xu
    // xyww sets depth to far plane (z = w, depth = 1.0); with GL_LEQUAL, skybox draws only behind scene geometry
    gl_Position = (u_Projection * viewNoTranslate * vec4(a_Position, 1.0)).xyww;
}

#type fragment
#version 450 core

in vec3 v_Direction;

uniform samplerCube u_Skybox;

out vec4 FragColor;

void main()
{
    vec3 color = texture(u_Skybox, normalize(v_Direction)).rgb;
    FragColor = vec4(color, 1.0f);
}