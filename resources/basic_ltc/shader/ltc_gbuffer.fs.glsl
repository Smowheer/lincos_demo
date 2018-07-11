#version 330

// both in W-space!
layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outPosition;

in vec3 w_normal;
in vec3 w_position;

void main() {
  // TODO normalize here or in vs?
  outNormal = vec4(normalize(w_normal), 1);
  outPosition = vec4(w_position, 1);
}
