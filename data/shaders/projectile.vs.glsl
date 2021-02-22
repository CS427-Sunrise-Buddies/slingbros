#version 330 

// Input attributes
in vec3 in_position;
in vec3 in_color;

out vec3 vcolor;
out vec2 vpos;
out vec3 worldPosition;

// Application data
uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	worldPosition = vec3(transform * vec4(in_position, 1.0));
	vpos = in_position.xy; // local coordinated before transform
	vcolor = in_color;
	gl_Position =  projection * view * transform * vec4(in_position, 1.0);
}