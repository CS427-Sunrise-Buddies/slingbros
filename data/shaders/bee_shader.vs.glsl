#version 330 

// Input attributes
in vec3 in_position;

// Application data
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_attrib_position;

void main()
{
	v_attrib_position = in_position;
	gl_Position =  projection * view * transform * vec4(in_position, 1.0);
}
