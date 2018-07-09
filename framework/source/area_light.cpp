#include "area_light.hpp"

AreaLight::AreaLight()
  : area_light_model{new groundPlane(0.0, 12.0)},
  light_position{glm::vec3(0.0, 10.0, 0.0)},
  rotation_x{0.0},
  rotation_y{0.0},
  scale_x{1.0},
  scale_y{1.0},
  light_intensity{5.0},
  diff_color{glm::vec3(1.0)},
  spec_color{glm::vec3(1.0)} {}

AreaLight::AreaLight(
    simpleModel* area_light_model,
    glm::fvec3 light_position,
    float rotation_x,
    float rotation_y,
    float scale_x,
    float scale_y,
    float light_intensity,
    glm::vec3 diff_color,
    glm::vec3 spec_color) 
  : area_light_model{area_light_model},
  light_position{light_position},
  rotation_x{rotation_x},
  rotation_y{rotation_y},
  scale_x{scale_x},
  scale_y{scale_y},
  light_intensity{light_intensity},
  diff_color{diff_color},
  spec_color{spec_color} {}
