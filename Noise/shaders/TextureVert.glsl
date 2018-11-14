#version 330 core

/// @brief MVP passed from app
uniform mat4 MVP;
// first attribute the vertex values from our VAO
layout (location=0) in vec3 inVert;
layout(location=1) in vec3 inNorm;
// second attribute the UV values from our VAO
layout (location=2)in vec2 inUV;
// we use this to pass the UV values to the frag shader
out vec3 vertUV;

void main()
{
// pre-calculate for speed we will use this a lot

// calculate the vertex position
gl_Position = MVP*vec4(inVert, 1.0);
// pass the UV values to the frag shader
vertUV=vec3(inUV.st,inNorm.z);
}
