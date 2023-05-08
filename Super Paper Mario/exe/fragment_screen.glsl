#version 330 core
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D outTexture11;

void main(){
	FragColor = texture(outTexture11,TexCoord);
	if(FragColor.a<0.2)	discard;
	
	//FragColor = vec4(1.0f,0.0f,0.0f,1.0f);
}