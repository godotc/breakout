#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D 	tex;
uniform vec3 texColor;

void main()
{
    vec4 origin = texture(tex, TexCoords); // format as GL_RED
    vec4 sampled = vec4( 1.f , 1.f ,1.f, origin.r);
    color = vec4(texColor, 1.f) * sampled;
}