#version 330

// Application data
uniform vec4 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	// Discard fragments with alpha less than threshold
	if (fcolor.a < 0.02) discard;

	color = fcolor;
}
