#version 330 core

out vec4 FragColor;

uniform int screenX;
uniform int screenY;
uniform float WaterLevel;
uniform float time;

void main() {
	float waveOffset = 0.025 * sin(gl_FragCoord.x / (screenX / 20.0) + time) + 0.025 * sin(gl_FragCoord.x / (screenX / 20.0) - 2 * time);
	float waveGlowOffset = waveOffset + 0.01;
	
	FragColor = ((gl_FragCoord.y / screenY) - waveGlowOffset) < WaterLevel 
			? ((gl_FragCoord.y / screenY) - waveOffset) < WaterLevel
				? vec4(0.0, 0.0, 1.0, 0.25)
				: vec4(0.0, 0.5, 1.0, 0.15)
			: vec4(0.0, 0.0, 0.0, 0.0);
}