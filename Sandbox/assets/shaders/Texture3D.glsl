#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in vec3 a_Normal;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out float v_TilingFactor;
out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
	v_Normal = a_Normal;

	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
	v_FragPos = a_Position;
}

#type fragment
#version 450 core

struct Material{
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;  
};

struct DirLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct FlashLight{
    vec3 position;
	vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in float v_TilingFactor;
in vec3 v_Normal;
in vec3 v_FragPos;

#define NR_POINT_LIGHTS 4  
uniform Material u_Material;
uniform PointLight u_PointLight[NR_POINT_LIGHTS];
uniform DirLight u_DirLight;
uniform FlashLight u_FlashLight;

uniform vec4 u_LightSourceColor;
uniform vec3 u_CameraPos;
uniform sampler2D u_Textures[32];
// uniform sampler2D u_Textures[16];

// PointLight
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// combine results
	vec3 ambient  = light.ambient * u_Material.diffuse * u_LightSourceColor.rgb;
    vec3 diffuse  = light.diffuse * diff * u_Material.diffuse * u_LightSourceColor.rgb;
    vec3 specular = light.specular * spec * u_Material.specular * u_LightSourceColor.rgb;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);

}

// DirLight
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	// combine results
	vec3 ambient  = light.ambient  * u_Material.diffuse;
    vec3 diffuse  = light.diffuse  * diff * u_Material.diffuse;
    vec3 specular = light.specular * spec * u_Material.specular;
    return (ambient + diffuse + specular);
}

void main()
{
	// TODO: u_TilingFactor

	// --------------------------------- Texture -----------------------------------------------------

    vec4 texColor = v_Color;

	switch(int(v_TexIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], v_TexCoord * v_TilingFactor); break;
		case  1: texColor *= texture(u_Textures[ 1], v_TexCoord * v_TilingFactor); break;
		case  2: texColor *= texture(u_Textures[ 2], v_TexCoord * v_TilingFactor); break;
		case  3: texColor *= texture(u_Textures[ 3], v_TexCoord * v_TilingFactor); break;
		case  4: texColor *= texture(u_Textures[ 4], v_TexCoord * v_TilingFactor); break;
		case  5: texColor *= texture(u_Textures[ 5], v_TexCoord * v_TilingFactor); break;
		case  6: texColor *= texture(u_Textures[ 6], v_TexCoord * v_TilingFactor); break;
		case  7: texColor *= texture(u_Textures[ 7], v_TexCoord * v_TilingFactor); break;
		case  8: texColor *= texture(u_Textures[ 8], v_TexCoord * v_TilingFactor); break;
		case  9: texColor *= texture(u_Textures[ 9], v_TexCoord * v_TilingFactor); break;
		case 10: texColor *= texture(u_Textures[10], v_TexCoord * v_TilingFactor); break;
		case 11: texColor *= texture(u_Textures[11], v_TexCoord * v_TilingFactor); break;
		case 12: texColor *= texture(u_Textures[12], v_TexCoord * v_TilingFactor); break;
		case 13: texColor *= texture(u_Textures[13], v_TexCoord * v_TilingFactor); break;
		case 14: texColor *= texture(u_Textures[14], v_TexCoord * v_TilingFactor); break;
		case 15: texColor *= texture(u_Textures[15], v_TexCoord * v_TilingFactor); break;
		case 16: texColor *= texture(u_Textures[16], v_TexCoord * v_TilingFactor); break;
		case 17: texColor *= texture(u_Textures[17], v_TexCoord * v_TilingFactor); break;
		case 18: texColor *= texture(u_Textures[18], v_TexCoord * v_TilingFactor); break;
		case 19: texColor *= texture(u_Textures[19], v_TexCoord * v_TilingFactor); break;
		case 20: texColor *= texture(u_Textures[20], v_TexCoord * v_TilingFactor); break;
		case 21: texColor *= texture(u_Textures[21], v_TexCoord * v_TilingFactor); break;
		case 22: texColor *= texture(u_Textures[22], v_TexCoord * v_TilingFactor); break;
		case 23: texColor *= texture(u_Textures[23], v_TexCoord * v_TilingFactor); break;
		case 24: texColor *= texture(u_Textures[24], v_TexCoord * v_TilingFactor); break;
		case 25: texColor *= texture(u_Textures[25], v_TexCoord * v_TilingFactor); break;
		case 26: texColor *= texture(u_Textures[26], v_TexCoord * v_TilingFactor); break;
		case 27: texColor *= texture(u_Textures[27], v_TexCoord * v_TilingFactor); break;
		case 28: texColor *= texture(u_Textures[28], v_TexCoord * v_TilingFactor); break;
		case 29: texColor *= texture(u_Textures[29], v_TexCoord * v_TilingFactor); break;
		case 30: texColor *= texture(u_Textures[30], v_TexCoord * v_TilingFactor); break;
		case 31: texColor *= texture(u_Textures[31], v_TexCoord * v_TilingFactor); break;
	}


	// --------------------------------- Light -----------------------------------------------------

	vec3 norm = normalize(v_Normal);
	vec3 viewDir = normalize(u_CameraPos - v_FragPos);
	// DirLight
	vec3 result;
	result = CalcDirLight(u_DirLight, norm, viewDir);
	// PointLight
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(u_PointLight[i], norm, v_FragPos, viewDir);

	result *= texColor.rgb;
	result = clamp(result, 0.0, 1.0);

    color = vec4(result, texColor.a);
}