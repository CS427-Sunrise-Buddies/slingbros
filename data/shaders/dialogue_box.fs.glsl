#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	vec4 texel = texture(sampler0, vec2(texcoord));

	// Discard fragments with alpha less than threshold
	if (texel.a < 0.1) discard;

	color = fcolor * texel;
}