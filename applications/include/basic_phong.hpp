#ifndef BASIC_PHONG_HPP
#define BASIC_PHONG_HPP

#include "application.hpp"

#include <glm/glm.hpp>

#include "helper.hpp"
#include "models.hpp"

class BasicPhong : public Application {
 public:
  // allocate and initialize objects
  BasicPhong(std::string const& resource_path);
  BasicPhong(BasicPhong const&) = delete;
  BasicPhong& operator=(BasicPhong const&) = delete;

  // draw all objects
  void render();

 protected:
  // common methods
  void initializeGUI();
  void initializeShaderPrograms();
  void initializeObjects();

  void renderScene() const;
  void resize() override;
  // render objects
  simpleQuad  quad;
  simpleModel teaPot;
  groundPlane plane;
  solidSphere sphere;
  simplePoint point;

  // frame buffer object
  Fbo fbo;

  // textures
  Tex diffuse;
  Tex normal;
  Tex position; 
  Tex depth;

  Timer timer;

  glm::fvec3 lightDir;
  float rotationAngle;
  float degreesPerSecond;
  float radius;

  bool autoRotate;
  bool debugShading;
  bool stencilCulling;
  bool debugStencil;
  bool clearDebugStencil;
  bool splatting;
};

#endif
