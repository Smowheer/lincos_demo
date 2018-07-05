#version 330
precision highp float;
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 w_normal;
out vec3 w_position;

void main() {
  mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

  vec4 hv_position = viewMatrix * modelMatrix * vec4(vPosition, 1);

  // outputs
  //v_normal = normalMatrix * vNormal;
  //v_position = hv_position.xyz / hv_position.w;
  w_normal = normalize(normalMatrix * vNormal);
  vec4 hw_position = modelMatrix * vec4(vPosition, 1);
  w_position = hw_position.xyz;

  gl_Position = projMatrix * hv_position;
}


