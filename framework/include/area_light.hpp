#ifndef AREA_LIGHT_HPP
#define AREA_LIGHT_HPP

#include <glm/glm.hpp>

class AreaLight{
  public:
  glm::vec3 light_position;
  float rotation_x;
  float rotation_y;
  float scale_x;
  float scale_y;
  float light_intensity;
  glm::vec3 diff_color;
  glm::vec3 spec_color;
  AreaLight();
  AreaLight(
      glm::fvec3 light_position,
      float rotation_x,
      float rotation_y,
      float scale_x,
      float scale_y,
      float light_intensity,
      glm::vec3 diff_color,
      glm::vec3 spec_color);
};

#endif
