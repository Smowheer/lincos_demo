#version 330

layout(location = 0) out vec4 o_color;

in vec3 v_normal;
in vec3 v_position;

uniform mat4 viewMatrix;

uniform vec3 pointLightPos;

void main() {
  vec3 nv_normal = normalize(v_normal);

  vec4 hv_pointLightPos = viewMatrix * vec4(pointLightPos, 1.0);
  vec3 v_pointLightPos = hv_pointLightPos.xyz / hv_pointLightPos.w;

  vec3 v_lightDir = normalize(v_pointLightPos - v_position);
  
  vec3 col = vec3(1.0, 1.0, 1.0) * max(0.0, dot(v_lightDir, nv_normal));
  o_color = vec4(col, 1.0);
  //o_color = vec4(nv_normal, 1.0);
  //o_color = vec4(v_lightDir,1.0);
}
