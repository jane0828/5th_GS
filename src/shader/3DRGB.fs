#version 410 core

uniform float alpha;

in vec3 to_color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(to_color, alpha);
}
