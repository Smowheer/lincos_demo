
#ifndef BASIC_LTC_HPP
#define BASIC_LTC_HPP

#include "application.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "helper.hpp"
#include "models.hpp"
#include "area_light.hpp"

class BasicLTC : public Application {
 public:
  // allocate and initialize objects
  BasicLTC(std::string const& resource_path);
  BasicLTC(BasicLTC const&) = delete;
  BasicLTC& operator=(BasicLTC const&) = delete;

  // draw all objects
  void render();

 protected:
  // common methods
  void initializeGUI();
  void initializeShaderPrograms();
  void initializeObjects();

  // own methods
  // forward version
  void render_forward();
  void render_light_forward(unsigned light_idx);
  void render_ltc_forward(unsigned light_idx);

  void draw_basic_scene(const std::string& current_shader);
  void resize() override;

  // render objects
  simpleQuad  quad;
  simpleModel teaPot;
  groundPlane plane;
  simplePoint point;

  // LTC Textures
  GLuint ltc_texture_1;
  GLuint ltc_texture_2;

  // ACES Framebuffer stuff
  GLuint rtt_framebuffer;
  GLuint depthbuffer;
  GLuint rtt_texture;

  // gui options
  std::vector<AreaLight> area_lights;

  float roughness;
};

#endif
