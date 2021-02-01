#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE THE WATER WAVE DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// The following is adapted from the example here: https://www.shadertoy.com/view/MdlXz8
	float tau = 1;
	int iterations = 14;
	
	vec2 p = vec2(modf(uv.x*tau, tau), modf(uv.y*tau, tau)) - vec2(150,150);
	vec2 i = vec2(p);
	float c = 1.0;
	float inten = .004;

	for (int n = 0; n < iterations; n++) 
	{
		float t = time/40.0f * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(iterations);
	c = 1.17-pow(c, 1.4);
	vec3 colour = vec3(pow(abs(c), 8.0));
    colour = clamp(colour + vec3(0.7, 0.7, 0.7), 0.0, 1.0);
	vec2 coord = clamp(vec2(uv.x - (colour.r - 0.7)/8.0, uv.y - (colour.g - 0.8)/8.0), 0.0, 1.0);
	

	//vec2 coord = vec2(mod(sin(uv.x), 1.0f), mod(sin(uv.y), 1.0f));
    return coord;
}

vec4 color_shift(vec4 in_color, vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE THE COLOR SHIFTING HERE (you may want to make it blue-ish)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// The following is adapted from the example here: https://www.shadertoy.com/view/MdlXz8
	float tau = 1;
	int iterations = 14;
	
	vec2 p = vec2(modf(uv.x*tau, tau), modf(uv.y*tau, tau)) - vec2(150,150);
	vec2 i = vec2(p);
	float c = 1.0;
	float inten = .004;

	for (int n = 0; n < iterations; n++) 
	{
		float t = time/40.0f * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(iterations);
	c = 1.17-pow(c, 1.4);
	vec3 colour = vec3(pow(abs(c), 8.0));
    colour = clamp(colour + vec3(0.7, 0.7, 0.7), 0.0, 1.0);
	colour *= in_color.xyz;
	return vec4(colour,1.0);
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