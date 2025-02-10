#version 330

uniform mat4 matVP;
uniform mat4 matGeo;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normalOS;

out vec3 normalWS;

void main() {
   normalWS = normalize(matGeo * vec4(normalOS, 0.0f)).xyz;
   gl_Position = matVP * matGeo * vec4(pos, 1);
}
