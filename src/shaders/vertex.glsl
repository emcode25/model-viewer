#version 460 core

out VS_OUT
{
	vec3 fragPos;
	vec3 lightPos;
	vec3 fragNormal;
	vec2 UV;
	vec3 tangentLightPos;
	vec3 tangentFragPos;
}
vs_out;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

uniform vec3 lightPosition;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	//Model-view matrix
	mat4 MV = view * model;
	mat3 normalMat = transpose(inverse(mat3(MV)));

	//Create inverse TBN matrix
	vec3 T = normalize(normalMat * inTangent);
	vec3 B = normalize(normalMat * inBitangent);
	vec3 N = normalize(normalMat * inNormal);
	mat3 TBN = transpose(mat3(T, B, N));

	//Create view-space vectors
	vs_out.fragPos = vec3(MV * vec4(inPos, 1.0));
	vs_out.lightPos = vec3(view * vec4(lightPosition, 1.0));
	vs_out.fragNormal = normalMat * inNormal;

	//Create tangent-space vectors
	vs_out.tangentFragPos = TBN * vs_out.fragPos;
	vs_out.tangentLightPos = TBN * vs_out.lightPos;
	
	vs_out.UV = inUV;

	gl_Position = proj * MV * vec4(inPos, 1.0);
}