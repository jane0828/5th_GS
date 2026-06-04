#version 410 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_color;
layout (location = 3) in vec2 a_tex_coord;

out vec2 to_tex_coord;

void main()
{
	gl_Position = vec4(a_pos, 1.0f);
	to_tex_coord = a_tex_coord;
}