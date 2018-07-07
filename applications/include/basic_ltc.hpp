
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
  // forward version
  void render_forward();
  void render_light_forward(unsigned light_idx);
  void render_ltc_forward(unsigned light_idx);

  // deferred version
  void render_deferred();
  void render_gbuffer();
  void render_ltc_deferred();
  void resize() override;

  // render objects
  simpleQuad  quad;
  simpleModel teaPot;
  groundPlane plane;
  simplePoint point;

  // LTC Textures
  GLuint ltc_texture_1;
  GLuint ltc_texture_2;


  bool bool_deferred;
  // Deferred Shading stuff
  Fbo gbuffer;
  //Tex tex_diffuse; // don't need this for now
  Tex tex_normal;
  Tex tex_position;
  Tex tex_depth;

  // ACES Framebuffer stuff
  GLuint rtt_framebuffer;
  GLuint depthbuffer;
  GLuint rtt_texture;

  // gui options
  struct AreaLight{
    glm::vec3 light_position;
    float rotation_x;
    float rotation_y;
    float scale_x;
    float scale_y;
    float light_intensity;
    glm::vec3 diff_color;
    glm::vec3 spec_color;
    AreaLight()
        : light_position{glm::vec3(0.0, 10.0, 0.0)},
          rotation_x{0.0},
          rotation_y{0.0},
          scale_x{1.0},
          scale_y{1.0},
          light_intensity{5.0},
          diff_color{glm::vec3(1.0)},
          spec_color{glm::vec3(1.0)} {
    }
    AreaLight(
        glm::fvec3 light_position,
        float rotation_x,
        float rotation_y,
        float scale_x,
        float scale_y,
        float light_intensity,
        glm::vec3 diff_color,
        glm::vec3 spec_color) 
        : light_position{light_position},
          rotation_x{rotation_x},
          rotation_y{rotation_y},
          scale_x{scale_x},
          scale_y{scale_y},
          light_intensity{light_intensity},
          diff_color{diff_color},
          spec_color{spec_color} {
    }
  };
  std::vector<AreaLight> area_lights;

  float roughness;

  // Configuration of shader algorithm
  bool clipless;

};

#endif
