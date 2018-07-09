
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
#include <iostream>
#include <sstream>

#include <deque>

#include "shader_loader.hpp"

void BasicLTC::render() {
  //std::cout << m_cam.viewDir.x << m_cam.viewDir.y << m_cam.viewDir.z << std::endl;

  render_forward();

  // draw final with ACES TODO understand ACES!
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shader("ltc_blit"));
  uniform("ltc_blit", "resolution", glm::vec2(resolution()));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rtt_texture);
  uniform("ltc_blit", "tex", 0);
  quad.draw();
  glUseProgram(0);
}

// FORWARD RENDERING faster than deferred here
void BasicLTC::render_forward() {
  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  // draw the colors to rtt_framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, rtt_framebuffer);
  {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw the lights themself
    for (unsigned int i = 0; i < area_lights.size(); ++i) {
      render_light_forward(i);
    }
    // blend all lighting calculations into rtt_texture
    for (unsigned int i = 0; i < area_lights.size(); ++i) {
      if (i == 1) {
        // after first draw blend the rest
        glEnable(GL_BLEND);
      }
      render_ltc_forward(i);
    }
  }
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BasicLTC::render_light_forward(unsigned int light_idx) {
  AreaLight l = area_lights[light_idx];
  std::vector<glm::vec4> points;
  int n_points = 0;
  for (auto v3 : l.area_light_model->vertices) {
    points.push_back(glm::vec4(v3.x, v3.y, v3.z, 1.0));
    n_points++;
  }
  // setup modelMatrix
  glm::mat4 modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, l.light_position);
  modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_y), glm::vec3(0.0,1.0,0.0));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_x), glm::vec3(1.0,0.0,0.0));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(l.scale_x, 1.0, l.scale_y));
  // render arealight[light_idx]
  glUseProgram(shader("arealight"));
  {
    // setup view and proj matrix
    uniform("arealight", "projMatrix", projectionMatrix());
    uniform("arealight", "viewMatrix", viewMatrix());
    uniform("arealight", "modelMatrix", modelMatrix);
    uniform("arealight", "u_color", l.diff_color);
    l.area_light_model->draw();

    // transform the points and
    // draw the points passed to the ltc shader
    glPointSize(2.5f);
    uniform("arealight", "u_color", glm::vec3(1.0, 0.0, 0.0));
    for (int j = 0; j < n_points; ++j) {
      points[j] = modelMatrix * points[j];
      points[j] = points[j] / points[j].a;
      glm::mat4 pModel = glm::translate(glm::mat4(1.0f), glm::vec3(points[j]));
      uniform("arealight", "modelMatrix", pModel);
      point.draw();
    }
  }
  glUseProgram(0);
}

void BasicLTC::render_ltc_forward(unsigned int light_idx) {
  AreaLight l = area_lights[light_idx];
  std::deque<glm::vec4> points;
  int n_points = (int)(l.area_light_model->vertices.size());
  for (int i = 1; i < n_points; ++i) {
    glm::vec3 v3 = l.area_light_model->vertices[i];
    if (flip_lights) {
      points.push_front(glm::vec4(v3.x, v3.y, v3.z, 1.0));
    } else {
      points.push_back(glm::vec4(v3.x, v3.y, v3.z, 1.0));
    }
  }
  points.push_front(glm::vec4(l.area_light_model->vertices[0], 1.0));



  // setup modelMatrix
  glm::mat4 modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, l.light_position);
  modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_y), glm::vec3(0.0,1.0,0.0));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_x), glm::vec3(1.0,0.0,0.0));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(l.scale_x, 1.0, l.scale_y));

  for (int j = 0; j < n_points; ++j) {
    points[j] = modelMatrix * points[j];
    points[j] = points[j] / points[j].a;
  }

  // render lighting
  glUseProgram(shader("ltc_forward"));
  {
    uniform("ltc_forward", "projMatrix", projectionMatrix());
    uniform("ltc_forward", "viewMatrix", viewMatrix());

    uniform("ltc_forward", "roughness", roughness);

    uniform("ltc_forward", "camera_position", m_cam.position);

    uniform("ltc_forward", "ltc_1", 0);
    uniform("ltc_forward", "ltc_2", 1);

    uniform("ltc_forward", "intensity", l.light_intensity);
    uniform("ltc_forward", "dcolor", l.diff_color);
    uniform("ltc_forward", "scolor", l.spec_color);

    uniform("ltc_forward", "n_points", n_points);
    for (int i = 0; i < n_points; ++i) {
      std::stringstream ss;
      ss << "points_arr[" << i << "]";
      uniform("ltc_forward", ss.str(), glm::vec3(points[i]));
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ltc_texture_2);

    // draw objects
    draw_basic_scene("ltc_forward");
  }
  glUseProgram(0);
}

// END OF FORWARD RENDERING

// SCENE DRAW

void BasicLTC::draw_basic_scene(const std::string& current_shader) {
    uniform(current_shader, "modelMatrix", glm::mat4(1.0));
    plane.draw();
    uniform(current_shader, "modelMatrix", glm::translate(glm::mat4(1.0), glm::vec3(3,0,0)));
    teaPot.draw();
    uniform(current_shader, "modelMatrix", glm::translate(glm::mat4(1.0), glm::vec3(3,0,3)));
    teaPot.draw();
}


BasicLTC::BasicLTC(std::string const& resource_path)
 :Application{resource_path + "basic_ltc/"}
 ,quad{}
 ,teaPot{m_resource_path + "../shared/data/teapot.obj"}
 ,plane{0.0f, 50.0f}
 ,point{}
 ,ltc_texture_1{0}
 ,ltc_texture_2{0}
 ,rtt_framebuffer{0}
 ,depthbuffer{0}
 ,rtt_texture{0}
 ,area_lights{
   {
     //AreaLight(
     //    new groundPlane(0.0, 2.4f),
     //    glm::vec3(-5.0, 5.0, 0.0),
     //    270.0f,
     //    90.0f,
     //    0.8f,
     //    0.8f,
     //    5.0f,
     //    glm::vec3(1.0, 0.2, 1.0),
     //    glm::vec3(1.0, 0.2, 1.0)),
     AreaLight(
         new GModel(),
         glm::vec3(4.5, 5.0,-3.0),
         180.0f,
         270.0f,
         0.5f,
         0.5f,
         15.0f,
         glm::vec3(0.0, 0.0, 1.0),
         glm::vec3(0.0, 0.0, 1.0)),
     AreaLight(
         new rModel(),
         glm::vec3(4.0, 5.0,-0.5),
         180.0f,
         270.0f,
         0.5f,
         0.5f,
         15.0f,
         glm::vec3(0.0, 1.0, 0.0),
         glm::vec3(0.0, 1.0, 0.0)),
     AreaLight(
         new aModel(),
         glm::vec3(4.0, 5.0, 1.5),
         180.0f,
         270.0f,
         0.5f,
         0.5f,
         15.0f,
         glm::vec3(1.0, 1.0, 0.0),
         glm::vec3(1.0, 1.0, 0.0)),
     AreaLight(
         new HModel(),
         glm::vec3(4.5, 5.0, 4.0),
         180.0f,
         270.0f,
         0.5f,
         0.5f,
         15.0f,
         glm::vec3(1.0, 0.5, 0.0),
         glm::vec3(1.0, 0.5, 0.0)),
     AreaLight(
         new SModel(),
         glm::vec3(4.5, 5.0, 6.5),
         180.0f,
         270.0f,
         0.5f,
         0.5f,
         15.0f,
         glm::vec3(1.0, 0.0, 0.0),
         glm::vec3(1.0, 0.0, 0.0)),
   }
 }
 ,roughness{0.85f}
 ,flip_lights{false}
{
  initializeGUI();
  initializeObjects();
  initializeShaderPrograms(); 
}

void BasicLTC::initializeGUI() {
  TwAddVarRW(tweakBar, "flip_lights", TW_TYPE_BOOLCPP, &flip_lights, "label='flip_lights'");
  TwAddVarRW(tweakBar, "roughness", TW_TYPE_FLOAT, &roughness, "label='roughness' min=0.05 step=0.001 max=1");
  for (unsigned int i = 0; i < area_lights.size(); ++i) {
    AreaLight& l = area_lights[i]; // reference!
    std::stringstream ss;
    ss << i;
    TwAddSeparator(tweakBar, ("sep" + ss.str()).c_str(), nullptr);
    TwAddVarRW(tweakBar, ("light_position" + ss.str()).c_str(), TW_TYPE_DIR3F, &(l.light_position), "label='light_position'");
    TwAddVarRW(tweakBar, ("rotation_x" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.rotation_x), "label='rotation_x' min=0 step=0.5 max=360");
    TwAddVarRW(tweakBar, ("rotation_y" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.rotation_y), "label='rotation_y' min=0 step=0.5 max=360");
    TwAddVarRW(tweakBar, ("scale_x" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.scale_x), "label='scale_x' min=0.001 step=0.001 max=5");
    TwAddVarRW(tweakBar, ("scale_y" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.scale_y), "label='scale_y' min=0.001 step=0.001 max=5");
    TwAddVarRW(tweakBar, ("light_intensity" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.light_intensity), "label='light_intensity' min=0.1 step=0.01 max=50");
    TwAddVarRW(tweakBar, ("diff_color" + ss.str()).c_str(), TW_TYPE_COLOR3F, &(l.diff_color), "label='diff_color'");
    TwAddVarRW(tweakBar, ("spec_color" + ss.str()).c_str(), TW_TYPE_COLOR3F, &(l.spec_color), "label='spec_color'");
  }
  TwAddSeparator(tweakBar, "sep123", nullptr);
}

// load shader programs
void BasicLTC::initializeShaderPrograms() {
  initializeShader("arealight",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/arealight.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/arealight.fs.glsl"}});

  initializeShader("ltc_forward",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc_forward.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc_forward.fs.glsl"}});

  initializeShader("ltc_blit",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc_blit.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc_blit.fs.glsl"}});
}

void SetClampedTextureState() {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void BasicLTC::initializeObjects() {
  // init objects
  glGenTextures(1, &ltc_texture_1);
  glGenTextures(1, &ltc_texture_2);
  glGenFramebuffers(1, &rtt_framebuffer);
  glGenTextures(1, &rtt_texture);
  glGenRenderbuffers(1, &depthbuffer);

  // init LTC textures
  glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, mat_data::g_ltc_1);
  SetClampedTextureState();
  glBindTexture(GL_TEXTURE_2D, ltc_texture_2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, mat_data::g_ltc_2);
  SetClampedTextureState();

  // init other objects
  resize();
}

void BasicLTC::resize() {
  // re-initialize objects for ACES
  glBindFramebuffer(GL_FRAMEBUFFER, rtt_framebuffer);
  // framebuffer texture
  glBindTexture(GL_TEXTURE_2D, rtt_texture);
  auto dims = resolution();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, dims.x, dims.y, 0, GL_RGBA, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // The depth buffer
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dims.x, dims.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
  // The color buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtt_texture, 0);
  // // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer incomplete" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#include "launcher.hpp"
// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<BasicLTC>(argc, argv);
}
