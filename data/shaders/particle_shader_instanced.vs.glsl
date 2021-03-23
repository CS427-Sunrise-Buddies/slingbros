#version 330 

// Input attributes
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 instanceColour;
layout (location = 2) in mat4 instanceTransform;

// Application data
uniform mat4 view;
uniform mat4 projection;

// Output to fragment shader
out vec4 instancedColour;

void main()
{
	instancedColour = instanceColour;
	gl_Position = projection * view * instanceTransform * vec4(in_position, 1.0);
}