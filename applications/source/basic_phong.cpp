#include "basic_phong.hpp"

// define M_PI on MSVC
#define _USE_MATH_DEFINES
#include <math.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <AntTweakBar.h>

#include <sstream>

#include "shader_loader.hpp"

void BasicPhong::render() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shader("pointlight"));
  uniform("pointlight", "viewMatrix", viewMatrix());
  uniform("pointlight", "projMatrix", projectionMatrix());

  glm::fmat4 modelMatrix = glm::fmat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, this->pointLightPos);
  modelMatrix = glm::scale(modelMatrix, glm::fvec3(0.1f));

  uniform("pointlight", "modelMatrix", modelMatrix);
  sphere.draw();

  glUseProgram(shader("phong"));
  uniform("phong", "viewMatrix", viewMatrix());
  uniform("phong", "projMatrix", projectionMatrix());
  uniform("phong", "pointLightPos", this->pointLightPos);

  for (int i = 0; i < 5; ++i) {
    std::stringstream ss;
    ss << "colors[" << i << "]";
    uniform("phong", ss.str().c_str(), glm::vec3(1.0/10.0, 1.0/5.0, 1.0/20.0));
  }

  uniform("phong", "modelMatrix", glm::fmat4(1.0f));
  teaPot.draw();

  uniform("phong", "modelMatrix", glm::fmat4(1.0f));
  plane.draw();

  glUseProgram(0);
}

BasicPhong::BasicPhong(std::string const& resource_path)
 :Application{resource_path + "basic_phong/"}
 ,quad{}
 ,teaPot{m_resource_path + "../shared/data/teapot.obj"}
 ,plane{0.0f, 12.0f}
 ,sphere{1, 20, 20}
 ,point{}
 ,pointLightPos{glm::fvec3(4.0f, 4.0f, 4.0f)}
{
  initializeGUI();
  initializeObjects();
  initializeShaderPrograms(); 
}

void BasicPhong::initializeGUI() {
  TwAddVarRW(tweakBar, "pointLightPos", TW_TYPE_DIR3F, &pointLightPos, "label='Point Light Position'");
}

// load shader programs
void BasicPhong::initializeShaderPrograms() {
  initializeShader("phong",{{GL_VERTEX_SHADER, m_resource_path + "./shader/phong.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/phong.fs.glsl"}});
  initializeShader("pointlight",{{GL_VERTEX_SHADER, m_resource_path + "./shader/pointlight.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/pointlight.fs.glsl"}});
}

void BasicPhong::initializeObjects() {
}

#include "launcher.hpp"
// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<BasicPhong>(argc, argv);
}
