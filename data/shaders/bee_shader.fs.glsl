#version 330

// Application data
uniform vec4 fcolor;

// Output color
layout(location = 0) out vec4 color;

in vec3 v_attrib_position;

void main()
{
	// Discard fragments with alpha less than threshold
	if (fcolor.a < 0.02) discard;

	// Discard fragments outside of quad center circle
	if(length(v_attrib_position) > 0.65) discard;

	// Colour two strips in the middle black
	if((-0.35 < v_attrib_position.x && v_attrib_position.x < -0.15)
	|| (0.15 < v_attrib_position.x && v_attrib_position.x < 0.35))
		color = vec4(0.0, 0.0, 0.0, fcolor.a);
	else 
		color = fcolor;
}
