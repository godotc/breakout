#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor = vec3(1,0,0);

uniform int hasTexture= 0;

void main()
{

	if(0 == hasTexture){
	 	 color = vec4(spriteColor, 1.0);
	}else{
		color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
	}
}
