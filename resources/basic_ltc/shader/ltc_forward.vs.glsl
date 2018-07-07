#version 330
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 w_normal;
out vec3 w_position;

void main() {
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1);

  mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
  w_normal = normalize(normalMatrix * vNormal);
  w_position = (modelMatrix * vec4(vPosition, 1)).xyz;
}
