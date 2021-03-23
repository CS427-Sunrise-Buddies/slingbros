#version 330

// Input from vertex shader
in vec4 instancedColour;

// Output color
layout(location = 0) out vec4 FragColour;

void main()
{
	// Discard fragments with alpha less than threshold
	if (instancedColour.a < 0.02) discard; 

	FragColour = instancedColour;
}
