#version 330 core

/// @brief MVP passed from app
uniform mat4 MVP;
uniform mat4 M;
uniform mat3 normalMatrix;
uniform vec3 cameraPos;
uniform int reflectOn;
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the normal passed in
layout (location = 1) in vec3 inNormal;
/// @brief the in uv
layout (location = 2) in vec2 inUV;
out int rOn;
// we use this to pass the UV values to the frag shader
out vec3 vertUV;

void main()
{
	vec4 position = M * vec4(inVert,1.0);
	vec3 normal = normalMatrix * inNormal;

	vec3 reflection = reflect(position.xyz - cameraPos, -normalize(normal));

	// calculate the vertex position
	gl_Position = MVP*vec4(inVert, 1.0);
	// pass the UV values to the frag shader
	if (reflectOn == 1)

		vertUV=vec3(reflection.x, -reflection.yz);
	else
		vertUV=position.xyz;
}
