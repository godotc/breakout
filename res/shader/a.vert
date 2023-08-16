#version 330 core
layout (location = 0 ) in vec4 vertex; // 2 postion 2 texCoord

out vec2 TexCoords;

uniform mat4 model = mat4(1.f);
uniform mat4 projection= mat4(1.f);


void main()
{
	TexCoords = vertex.zw;
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

