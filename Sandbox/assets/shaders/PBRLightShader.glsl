#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in ivec4 a_BoneIDs;
layout(location = 6) in vec4 a_Weights;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec2 v_TexCoord;
out vec3 v_Tangent;
out vec3 v_Bitangent;
flat out ivec4 v_BoneIDs;
out vec4 v_Weights;

out vec3 v_FragPos;

void main()
{
	v_Normal = a_Normal;
	v_TexCoord = a_TexCoords;
	v_Tangent = a_Tangent;
	v_Bitangent = a_Bitangent;
	v_BoneIDs = a_BoneIDs;
	v_Weights = a_Weights;

	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
	v_FragPos = a_Position;
}

#type fragment
#version 450 core

struct Material{
	vec3 diffuse;
	vec3 normal;
	float metallic;				// from specular
	vec3 emissive;
	float alpha;				// from opacity
	float height;

	float occlusion; 			// from ambient
	float roughness;			// from shininess
	float displacement;
	vec3 light;
	vec3 reflection;
};

struct Light{
    vec3 position;
	vec3 color;
	float intensity;
	int type; 			// 0 = Parallel light, 1 = Point source light
};

layout(location = 0) out vec4 color;

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec3 v_Tangent;
in vec3 v_Bitangent;
flat in ivec4 v_BoneIDs;
in vec4 v_Weights;

in vec3 v_FragPos;

#define NR_LIGHTS 10
Material g_Material;
uniform Light u_Lights[NR_LIGHTS];
uniform int u_LightCount;

uniform vec3 u_CameraPos;
uniform sampler2D u_Textures[32];
// uniform samplerCube u_PrefilterMap;  // Environment Map
// uniform sampler2D u_BRDFLUT;         // BRDF LUT

#define PI 3.1415926

// Texture Sampler
void ProcessMaterialTextures(vec2 texCoord, float tilingFactor)
{
	g_Material.diffuse 			= texture(u_Textures[ 1], v_TexCoord * tilingFactor).rgb; 			// diffuse
	g_Material.normal 			= texture(u_Textures[ 2], v_TexCoord * tilingFactor).rgb; 			// normal
	g_Material.metallic 		= texture(u_Textures[ 3], v_TexCoord * tilingFactor).r; 			// from specular
	g_Material.emissive 		= texture(u_Textures[ 4], v_TexCoord * tilingFactor).rgb; 			// emissive
	g_Material.alpha 			= texture(u_Textures[ 5], v_TexCoord * tilingFactor).r; 			// from opacity
	g_Material.height 			= texture(u_Textures[ 6], v_TexCoord * tilingFactor).r; 			// height
	g_Material.occlusion 		= texture(u_Textures[ 7], v_TexCoord * tilingFactor).r; 			// from ambient
	g_Material.roughness 		= texture(u_Textures[ 8], v_TexCoord * tilingFactor).r; 			// from shininess
	g_Material.displacement 	= texture(u_Textures[ 9], v_TexCoord * tilingFactor).r; 			// displacement
	g_Material.light 			= texture(u_Textures[10], v_TexCoord * tilingFactor).rgb; 			// light map
	g_Material.reflection 		= texture(u_Textures[11], v_TexCoord * tilingFactor).rgb; 			// reflection

	// ------------------------ text --------------------------------
	// g_Material.diffuse = vec3(1.0, 1.0, 1.0);  // white
    // g_Material.normal = vec3(0.5, 0.5, 1.0);   // 默认法线（切线空间中的z方向）
    // g_Material.metallic = 0.0;
    // g_Material.emissive = vec3(0.0);
    // g_Material.alpha = 1.0;
    // g_Material.height = 0.0;
    // g_Material.occlusion = 1.0;
    // g_Material.roughness = 0.5;
    // g_Material.displacement = 0.0;
    // g_Material.light = vec3(0.0);
    // g_Material.reflection = vec3(0.0);
}

// ------------------ Auxiliary Function ------------------ //
// GGX Normal Distribution Function
float DistributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = NdotH2 * (a2 - 1) + 1;
	denom = PI * pow(denom, 2.0);

	return nom / denom;
}

// Fresnel Equation - Schlick approximation
vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	return F0 + (1 - F0) * pow(1 - HdotV, 5.0);
}

// Fresnel with roughness (for ambient light)
vec3 fresnelSchlickRoughness(float HdotV, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1 - HdotV, 5.0);
}

// Geometric Function
float GeometrySchlickGGX(float NdotV, float k)
{
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return nom / denom;
}
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float k_dir = pow(roughness + 1.0, 2.0) / 8.0;
	// float k_IBL = pow(roughness, 2.0) / 2.0;

	// direction light
	float ggx1 = GeometrySchlickGGX(NdotV, k_dir);
	float ggx2 = GeometrySchlickGGX(NdotL, k_dir);

	// environment light
	// float ggx21 = GeometrySchlickGGX(NdotV, k_IBL);
	// float ggx22 = GeometrySchlickGGX(NdotL, k_IBL);

	return ggx1 * ggx2;
}

// CalcFinal Normal
vec3 CalcFinalNormal()
{
	// 1. From normal map sampling - tangent space
	vec3 tangentNormal = g_Material.normal;
	tangentNormal = normalize(tangentNormal * 2.0 - 1.0);	// [0, 1] -> [-1, 1]

	// 2. Build the TBN Matrix
	mat3 TBN = mat3(
		normalize(v_Tangent),
		normalize(v_Bitangent),
		normalize(v_Normal)
	);

	// 3. Switch to the world coordinate system
	vec3 worldNormal = normalize(TBN * tangentNormal);

	return worldNormal;
}

// ------------------ Rendering Equation ------------------ //
// Direct illumination part (discrete summation)
vec3 CalcDirectLighting(Light light, vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness)
{
	// Direction of light
	vec3 L = light.type == 0 ? 
		normalize(-light.position) : 
		normalize(light.position - v_FragPos);
	vec3 H = normalize(V + L);

	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float HdotV = max(dot(H, V), 0.0);

	// 1. NDF - GGX
	float D = DistributionGGX(NdotH, roughness);

	// 2. Geometric function - Smith with direct lighting k
	float G = GeometrySmith(NdotV, NdotL, roughness);

	// 3. Fresnel equation - Schlick
	vec3 F = fresnelSchlick(HdotV, F0);

	// 4. Combine into BRDF
	// - specular BRDF
	vec3 numerator = D * G * F;
	float denominator = 4.0 * NdotV * NdotL + 0.0001;
	vec3 specularBRDF = numerator / denominator;
	// - diffuse reflection BRDF
	vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
	vec3 diffuseBRDF = kD * albedo / PI;

	// 5. Complete BRDF
	vec3 BRDF = diffuseBRDF + specularBRDF;

	// 6. Light source radiance
	float attenuation = 1.0;
	if(light.type == 1)
	{
		float distance = length(light.position - v_FragPos);
		attenuation /= (distance * distance);
	}
	
	vec3 radiance = light.color * light.intensity * attenuation;

	// 7. The direct illumination part of the reflection equation
	return BRDF * radiance * NdotL;
}

// Environmental lighting part (pre-computed integration)
vec3 CalculateIBL(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    
    // 1. Fresnel term - using roughness
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // 2. Diffuse ambient light - illuminanc
	vec3 ambientColor = vec3(0.03);
    vec3 diffuse = ambientColor * albedo;
    
    // 3. Mirror reflection of ambient light - (pre-filtering + BRDF integration)
    vec3 R = reflect(-V, N);
    
    // Pre-filtered environment map sampling - selecting mip level based on roughness
    // const float MAX_REFLECTION_LOD = 4.0;
    // vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    
    // BRDF LUT Sampler
    // vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    // vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 specular = F * 0.1; // Fixed environmental reflection intensity
    
    // 4. Combined environmental lighting
    return kD * diffuse + specular;
}

void main()
{
	// TODO: u_TilingFactor
	float tilingFactor = 1.0;

	// 1. Sampler texture
    // vec4 texColor = texture(u_Textures[ 0], v_TexCoord * tilingFactor); 	// white texture
	ProcessMaterialTextures(v_TexCoord, tilingFactor);
	
	// 2. Calculate Basic parameters
	// vec3 finalNormal = CalcFinalNormal();
	vec3 finalNormal = normalize(v_Normal);
	vec3 viewDir = normalize(u_CameraPos - v_FragPos);

	// 3. Calculate F0
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, g_Material.diffuse, g_Material.metallic);
	// F0 = vec3(0.03);

	// 4. Reflection equation: Direct illumination part (discrete summation)
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < u_LightCount; ++i)
		Lo += CalcDirectLighting(
			u_Lights[i], 
			finalNormal, viewDir, F0, 
			g_Material.diffuse, g_Material.metallic, g_Material.roughness
			);
	
	// 5. Reflection equation: Environmental illumination component (pre-computed integration)
	vec3 ambient = CalculateIBL(
		finalNormal, viewDir, F0, 
		g_Material.diffuse, g_Material.metallic, g_Material.roughness
		);
	
	// 6. Application of ambient light occlusion
	ambient *= g_Material.occlusion;

	// 7. Merge all light contributions
	vec3 result = ambient + Lo + g_Material.emissive;
	// vec3 result = ambient + g_Material.emissive;	// test-line

	// 8. Color mapping and gamma correction
    result = result / (result + vec3(1.0)); 	// Reinhard tone mapping
    // result = pow(result, vec3(1.0/2.2));      	// Gamma correction
	
    color = vec4(result, g_Material.alpha);
    // color = vec4(v_TexCoord, 0.0, 1.0);
	// color = vec4(g_Material.metallic, g_Material.roughness, 0.0, 1.0);
    // color = vec4(1.0, 0.0, 0.0, 1.0);
    // color = vec4(texture(u_Textures[1], v_TexCoord).rgb, 1.0);
}