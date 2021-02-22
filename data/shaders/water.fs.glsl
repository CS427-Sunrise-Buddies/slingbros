#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// TODO any second pass shader distortion
	// May want to implement weather effects here later
    return uv;
}

vec4 color_shift(vec4 in_color, vec2 uv) 
{
	// TODO any second pass shader color shifting
	// May want to implement weather effects here later
	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	vec4 color = in_color;
	if (darken_screen_factor > 0)
		color -= darken_screen_factor * vec4(0.5, 0.5, 0.5, 0);

	return color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color, coord);
    color = fade_color(color);
}