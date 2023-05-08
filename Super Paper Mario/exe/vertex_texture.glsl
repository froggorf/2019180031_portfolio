#version 330 core
layout (location = 3) in vec3 vPos;
layout (location = 4) in vec3 vNormal;
layout (location = 5) in vec2 vTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main(){
	gl_Position = projection *view * model * vec4(vPos,1.0);
	FragPos = vec3(model * vec4(vPos,1.0));
	Normal = vNormal;
	TexCoord = vTexCoord;
	
}