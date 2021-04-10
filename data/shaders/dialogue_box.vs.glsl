#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

void main()
{
	texcoord = in_texcoord;
	gl_Position =  vec4(in_position, 1.0);
}