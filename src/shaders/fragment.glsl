#version 460 core

#define MAX_TEXTURES 16

in VS_OUT
{
	vec3 fragPos;
	vec3 lightPos;
	vec3 fragNormal;
	vec2 UV;
	vec3 tangentLightPos;
	vec3 tangentFragPos;
}
fs_in;

layout(location = 0) out vec4 fragColor;

uniform int diffuseSize;
uniform int specularSize;
uniform int normalSize;
uniform sampler2D diffuseTex[MAX_TEXTURES];
uniform sampler2D specularTex[MAX_TEXTURES];
uniform sampler2D normalTex[MAX_TEXTURES];

uniform bool enableLighting;
uniform vec3 lightColor;

uniform bool enableNormalMapping;

//All lighting calculations are done in VIEW space
void main()
{
	vec3 diffuseSample = vec3(texture(diffuseTex[0], fs_in.UV));

	//Skip phong lighting?
	if(!enableLighting)
	{
		fragColor = vec4(diffuseSample, 1.0);
		return;
	}

	vec3 lightDir, viewDir, normal;
	if(enableNormalMapping)
	{
		lightDir = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos);
		viewDir = normalize(-fs_in.tangentFragPos);

		normal = texture(normalTex[0], fs_in.UV).rgb;
		normal = normalize(normal * 2.0 - 1.0); //Convert from [0,1] to [-1,1]
	}
	else
	{
		lightDir = normalize(fs_in.lightPos - fs_in.fragPos);
		viewDir = normalize(-fs_in.fragPos);
		normal = fs_in.fragNormal;
	}

	//Diffuse step
	vec3 norm = normalize(normal);

	float diff = max(0.0, dot(norm, lightDir));
	vec3 diffuse = diff * lightColor * diffuseSample;

	//Specular step
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specularSample = vec3(texture(specularTex[0], fs_in.UV));
	vec3 specular = spec * lightColor * specularSample;

	fragColor = vec4(diffuse + specular, 1.0);
}