#ifndef AREA_LIGHT_HPP
#define AREA_LIGHT_HPP

#include <glm/glm.hpp>

#include "models.hpp"

class AreaLight{
  public:
  // shape model with vertices in cw order
  simpleModel* area_light_model;

  // don't remove those components so tweakbar can use them
  glm::vec3 light_position;
  float rotation_x;
  float rotation_y;
  float scale_x;
  float scale_y;
  //

  float light_intensity;
  glm::vec3 diff_color;
  glm::vec3 spec_color;
  AreaLight();
  AreaLight(
      simpleModel* area_light_model,
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
