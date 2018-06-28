#version 330
layout(location = 0) in vec3 vPosition;

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0);
}
