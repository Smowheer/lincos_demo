
#ifndef BASIC_LTC_HPP
#define BASIC_LTC_HPP

#include "application.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "helper.hpp"
#include "models.hpp"

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
  void renderScene();
  void resize() override;

  // TODO maybe this is bs
  unsigned int render_clear_count;
  unsigned int render_clear_modulo;

  // render objects
  simpleQuad  quad;
  simpleModel teaPot;
  groundPlane plane;
  solidSphere sphere;
  simplePoint point;

  // Textures
  GLuint ltc_texture_1;
  GLuint ltc_texture_2;

  // ACES Framebuffer stuff
  GLuint rtt_framebuffer;
  GLuint depthbuffer;
  GLuint rtt_texture;

  // gui options
  glm::vec3 light_position;
  float rotation_x;
  float rotation_y;
  float scale_x;
  float scale_y;
  float light_intensity;
  glm::vec3 diff_color;
  glm::vec3 spec_color;

  float roughness;

  // Configuration of shader algorithm
  bool clipless;

};

#endif
