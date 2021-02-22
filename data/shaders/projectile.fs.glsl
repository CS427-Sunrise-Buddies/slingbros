#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 vpos; // Distance from local origin
in vec3 worldPosition;

// Application data
uniform float time;
uniform vec3 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float radius = 1.0;

	float dx = vpos.x;
	float dy = vpos.y;
	float dz = sqrt(radius*radius - dx*dx - dy*dy);

	vec3 L = normalize(vec3(1.0, 1.0, 1.0));
	vec3 N = normalize(vec3(dx, dy, dz));

	float i = max(dot(N, L), 0.0);
	float t = 0.1 * sin(time) + 1.0;

	vec3 yellow = vec3(1.0, 1.0, 0.0);
	vec3 orange = vec3(1.0, 0.27, 0.0);
	vec3 color1 = orange * i;
	vec3 color2 = yellow * (1.0 - i);
	color = vec4(vcolor + (color1 + color2) * t, 1.0);
}