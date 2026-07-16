#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_Direction;

void main()
{
    v_Direction = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

in vec3 v_Direction;
uniform samplerCube u_EnvironmentMap;

out vec4 FragColor;

const float PI = 3.1415926;

void main()
{
    vec3 N = normalize(v_Direction);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    vec3 irradiance = vec3(0.0);
    float sampleDelta = 0.025;
    uint samples = 0;

    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta)
            );

            vec3 sampleDir = tangentSample.x * right
                            + tangentSample.y * up
                            + tangentSample.z * N;

            irradiance += texture(u_EnvironmentMap, sampleDir).rgb * cos(theta) * sin(theta);

            samples++;
        }
    }

    irradiance = PI * irradiance / float(samples);

    FragColor = vec4(irradiance, 1.0);
}