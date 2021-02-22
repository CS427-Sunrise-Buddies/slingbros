#version 330 

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	texcoord = in_texcoord;
	gl_Position =  projection * view * transform * vec4(in_position, 1.0);
}
