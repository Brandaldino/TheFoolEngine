#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in vec3 a_Normal;

// uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out float v_TilingFactor;
out vec3 v_Normal;
// out vec3 v_FragPos;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
	v_Normal = a_Normal;

	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec3 v_Normal;

uniform vec4 u_LightSourceColor;

void main()
{
    color = v_Color * u_LightSourceColor;
}