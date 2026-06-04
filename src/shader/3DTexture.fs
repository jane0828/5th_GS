#version 410 core

uniform sampler2D texture1;

in vec2 to_tex_coord;

out vec4 FragColor;

void main()
{
	FragColor = texture(texture1, to_tex_coord);
}