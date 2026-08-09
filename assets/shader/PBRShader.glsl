#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_BitTangent;
layout(location = 4) in vec3 a_Normal;
layout(location = 5) in ivec4 a_BoneIDs;
layout(location = 6) in vec4 a_Weights;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

out vec2 v_TexCoord;
out vec3 v_Tangent;
out vec3 v_BitTangent;
out vec3 v_Normal;
flat out ivec4 v_BoneIDs;
out vec4 v_Weights;

out vec3 v_FragPos;

void main()
{
    v_TexCoord = a_TexCoord;
    v_Tangent = normalize(mat3(u_Model) * a_Tangent);
    v_BitTangent = normalize(mat3(u_Model) * a_BitTangent);
    v_Normal = normalize(mat3(u_Model) * a_Normal);
    v_BoneIDs = a_BoneIDs;
    v_Weights = a_Weights;

    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
    v_FragPos = (u_Model * vec4(a_Position, 1.0)).xyz;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec3 v_Tangent;
in vec3 v_BitTangent;
in vec3 v_Normal;
flat in ivec4 v_BoneIDs;
in vec4 v_Weights;

in vec3 v_FragPos;

#define PI 3.1415926
#define NR_LIGHTS 10
#define LIGHT_TYPE_DIRECTION 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define LIGHT_TYPE_RECT 3
#define LIGHT_TYPE_DISK 4
#define MAX_SHADOW_LIGHTS 4

struct GPU_Light
{
    vec4 Position; // xyz = pos (point/spot), w = range
    vec4 Direction; // xyz = dir(direction/spot)
    vec4 Color; // xyz = color, w = intensity
    vec4 Params; // x = range, y = innerCos, z = outerCos, w = 
    int ShadowIndex;
};

layout(std140, binding = 2) uniform LightBlock
{
    GPU_Light u_Lights[NR_LIGHTS];
    int u_LightCount;
};

uniform vec3 u_CameraPos;
uniform sampler2D u_Textures[32];

uniform vec3  u_AlbedoFactor;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;
uniform float u_AOStrength;
uniform mat4 u_ShadowMatrices[MAX_SHADOW_LIGHTS];

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D  u_BRDFLUT;
uniform sampler2DArray  u_ShadowMaps; // slot 8


// ============= PBR Functions =============
float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return nom / denom;
}

vec3 fresnelSchlick(float HdotV, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);
}

vec3 fresnelSchlickRoughness(float HdotV, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HdotV, 5.0);
}

float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

vec3 CalcFinalNormal(vec3 tangentNormal)
{
    vec3 tn = normalize(tangentNormal * 2.0 - 1.0);
    mat3 TBN = mat3(normalize(v_Tangent), normalize(v_BitTangent), normalize(v_Normal));
    return normalize(TBN * tn);
}

vec3 CalculateIBL(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness, float occlusion)
{
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 R = reflect(-V, N);
    vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * 4.0).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;

    vec3 specular = prefiltered * (F * brdf.x + brdf.y);


    return (kD * diffuse + specular) * occlusion;
}

// Shadow
float CalculateShadow(vec4 shadowCoord, int layer)
{
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;  // perspective divide
    projCoords = projCoords * 0.5 + 0.5;    // The prerequisite is that there is no offset in the bias.

    float currentDepth = projCoords.z;
    float shadow = 0.0;
    float bias = 0.005;

    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMaps, 0).xy);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec3 albedo = texture(u_Textures[1], v_TexCoord).rgb * u_AlbedoFactor;

    vec3 tangentNormal = texture(u_Textures[2], v_TexCoord).rgb;
    vec3 N = (length(v_Tangent) < 0.001) ? normalize(v_Normal) : CalcFinalNormal(tangentNormal);

    vec4 mr = texture(u_Textures[3], v_TexCoord);
    float roughness = mr.g * u_RoughnessFactor;
    float metallic = mr.b * u_MetallicFactor;

    float occlusion = 1.0;
    vec4 ao = texture(u_Textures[4], v_TexCoord);
    occlusion = ao.r * u_AOStrength;

    vec3 V = normalize(u_CameraPos - v_FragPos);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_LightCount; ++i)
    {
        GPU_Light gl = u_Lights[i];
        int type = int(gl.Params.w);

        vec3 L;
        float attenuation = 1.0;

        if(type == LIGHT_TYPE_DIRECTION)
            L = normalize(gl.Direction.xyz);
        else if(type == LIGHT_TYPE_POINT)
        {
            vec3 delta = gl.Position.xyz - v_FragPos;
            float dist = length(delta);
            if(dist > gl.Position.w)
                continue;
            L = delta / dist;
            attenuation = 1.0 / (dist * dist);
        }
        else if(type == LIGHT_TYPE_SPOT)
        {
            vec3 delta = gl.Position.xyz - v_FragPos;
            float dist = length(delta);
            if(dist > gl.Position.w)
                continue;
            L = delta / dist;

            float theta = dot(L, normalize(-gl.Direction.xyz));
            float epsilon = gl.Params.y - gl.Params.z; // innerCos - outerCos
            float spotFactor = clamp((theta - gl.Params.z) / epsilon, 0.0, 1.0);
            if(spotFactor < 0.001)
                continue;

            attenuation = spotFactor * spotFactor / (dist * dist);
        }
        else
            continue;

        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float D = DistributionGGX(NdotH, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        vec3 F = fresnelSchlick(HdotV, F0);

        vec3 specularBRDF = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 kD = (1.0 - F) * (1.0 - metallic);
        vec3 diffuseBRDF = kD * albedo / PI;

        vec3 radiance = gl.Color.xyz * gl.Color.w * attenuation;
        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL;

        if(gl.ShadowIndex >= 0)
        {
            vec4 shadowCoord = u_ShadowMatrices[gl.ShadowIndex] * vec4(v_FragPos, 1.0);

            float shadow = CalculateShadow(shadowCoord, gl.ShadowIndex);
            Lo *= (1.0 - shadow);
        }
    }

    vec3 ambient = CalculateIBL(N, V, F0, albedo, metallic, roughness, occlusion);
    vec3 result = ambient + Lo;
    result = result / (result + vec3(1.0));
    color = vec4(result, 1.0);

    // === Test Color ==============
    // color = vec4(1.0, 0.0, 0.0, 1.0);
}
