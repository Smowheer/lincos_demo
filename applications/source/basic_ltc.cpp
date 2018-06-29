
#include "basic_ltc.hpp"

// define M_PI on MSVC
#define _USE_MATH_DEFINES
#include <math.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <AntTweakBar.h>

#include "shader_loader.hpp"

void BasicLTC::render() {
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

  uniform("phong", "modelMatrix", glm::fmat4(1.0f));
  teaPot.draw();

  uniform("phong", "modelMatrix", glm::fmat4(1.0f));
  plane.draw();

  glUseProgram(0);
}

BasicLTC::BasicLTC(std::string const& resource_path)
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

void BasicLTC::initializeGUI() {
}

// load shader programs
void BasicLTC::initializeShaderPrograms() {
  initializeShader("ltc",{{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc.fs.glsl"}});
  initializeShader("arealight",{{GL_VERTEX_SHADER, m_resource_path + "./shader/arealight.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/arealight.fs.glsl"}});
}

void BasicLTC::initializeObjects() {
}

#include "launcher.hpp"
// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<BasicLTC>(argc, argv);
}
