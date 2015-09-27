#version 330 core
// this is a pointer to the current 2D texture object
uniform samplerCube cubeMap;
// the vertex UV
in vec3 vertUV;
// the final fragment colour
layout (location =0) out vec4 outColour;
void main ()
{
 // set the fragment colour to the current texture
 outColour = texture(cubeMap,vertUV);
}
