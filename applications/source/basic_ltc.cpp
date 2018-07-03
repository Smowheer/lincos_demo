
#include "basic_ltc.hpp"

#include "basic_ltc_matdata.hpp"

// define M_PI on MSVC
#define _USE_MATH_DEFINES
#include <math.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <AntTweakBar.h>

#include <vector>

#include "shader_loader.hpp"

void BasicLTC::render() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0,3.0,-5.0));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0,0.0,0.0));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

  glUseProgram(shader("arealight"));

  // setup view and proj matrix
  uniform("arealight", "viewMatrix", viewMatrix());
  uniform("arealight", "projMatrix", projectionMatrix());

  // draw the area light
  uniform("arealight", "modelMatrix", modelMatrix);
  uniform("arealight", "u_color", glm::vec3(1.0));
  plane.draw();

  // draw the points passed to the ltc shader
  glPointSize(10.0f);

  std::vector<glm::vec4> points = {
   glm::vec4(-12.0f, 0.0f, -12.0f, 1.0f),
   glm::vec4(-12.0f, 0.0f, 12.0f, 1.0f),
   glm::vec4(12.0f, 0.0f, 12.0f, 1.0f),
   glm::vec4(12.0f, 0.0f, -12.0f, 1.0f)
  };
  std::vector<glm::vec3> colors = {
    glm::vec3(1.0,0.0,0.0),
    glm::vec3(0.0,1.0,0.0),
    glm::vec3(0.0,0.0,1.0),
    glm::vec3(1.0,1.0,0.0)
  };
  for (int i = 0; i < 4; ++i) {
    points[i] = modelMatrix * points[i];
    points[i] = points[i] / points[i].a;
    glm::mat4 pModel = glm::translate(glm::mat4(1.0f), glm::vec3(points[i]));
    uniform("arealight", "modelMatrix", pModel);
    uniform("arealight", "u_color", colors[i]);
    point.draw();
  }

  //for (int i = 0; i < 4; ++i) {
  //  points[i] = viewMatrix() * points[i];
  //  points[i] = points[i] / points[i].a;
  //}

  // draw the ground
  glUseProgram(shader("ltc"));
  uniform("ltc", "modelMatrix", glm::mat4(1.0));
  uniform("ltc", "viewMatrix", viewMatrix());
  uniform("ltc", "projMatrix", projectionMatrix());
  uniform("ltc", "p1", glm::vec3(points[0]));
  uniform("ltc", "p2", glm::vec3(points[1]));
  uniform("ltc", "p3", glm::vec3(points[2]));
  uniform("ltc", "p4", glm::vec3(points[3]));
  uniform("ltc", "ltc_1", 0);
  uniform("ltc", "ltc_2", 1);
  uniform("ltc", "camera_position", m_cam.position);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_2);
  plane.draw();

  glUseProgram(0);
}

BasicLTC::BasicLTC(std::string const& resource_path)
 :Application{resource_path + "basic_ltc/"}
 ,quad{}
 ,teaPot{m_resource_path + "../shared/data/teapot.obj"}
 ,plane{0.0f, 12.0f}
 ,sphere{1, 20, 20}
 ,point{}
 ,ltc_texture_1{0}
 ,ltc_texture_2{0}
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

void SetClampedTextureState() {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void BasicLTC::initializeObjects() {
  glGenTextures(1, &ltc_texture_1);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, mat_data::g_ltc_1);
  SetClampedTextureState();

  glGenTextures(1, &ltc_texture_2);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, mat_data::g_ltc_2);
  SetClampedTextureState();
}

#include "launcher.hpp"
// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<BasicLTC>(argc, argv);
}
