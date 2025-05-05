#version 330 core

in vec3 TexCoord;

out vec4 FragColor;

uniform bool isWet;
uniform samplerCube texture1;

void main() {
	float wetnessOpacity = 0.3;
	FragColor = mix(texture(texture1, TexCoord), vec4(0.0, 0.5, 1.0, 1.0), (isWet ? wetnessOpacity : 0.0));
}