#version 330 

// Input attributes
in vec3 in_position;

// Application data
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position =  projection * view * transform * vec4(in_position, 1.0);
}
