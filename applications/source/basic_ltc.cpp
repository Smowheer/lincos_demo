
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

#include "shader_loader.hpp"

void BasicLTC::render() {
  // draw the geometry to gbuffer
  gbuffer.bind();
  {
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    render_gbuffer();
  }
  gbuffer.unbind();
  
  // draw the colors to rtt_framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, rtt_framebuffer);
  {
    // render with gbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    render_ltc_quad();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

void BasicLTC::render_gbuffer() {
  // draw the ground
  glUseProgram(shader("ltc_gbuffer"));

  // vertex shader uniforms
  uniform("ltc_gbuffer", "modelMatrix", glm::mat4(1.0));
  uniform("ltc_gbuffer", "viewMatrix", viewMatrix());
  uniform("ltc_gbuffer", "projMatrix", projectionMatrix());

  // draw ground
  plane.draw();

  // draw the pot ( uniforms stay the same :) )
  teaPot.draw();

  glUseProgram(0);
}

void BasicLTC::render_ltc_quad() {
  // setup global stuff
  std::vector<glm::vec4> points = {
   glm::vec4(-12.0f, 0.0f, -12.0f, 1.0f),
   glm::vec4(-12.0f, 0.0f, 12.0f, 1.0f),
   glm::vec4(12.0f, 0.0f, 12.0f, 1.0f),
   glm::vec4(12.0f, 0.0f, -12.0f, 1.0f)
  };
  std::vector<glm::vec4> current_points(4, glm::vec4(0.0));
  std::vector<glm::vec3> colors = {
    glm::vec3(1.0,0.0,0.0),
    glm::vec3(0.0,1.0,0.0),
    glm::vec3(0.0,0.0,1.0),
    glm::vec3(1.0,1.0,0.0)
  };
  // set global uniforms
  glUseProgram(shader("arealight"));
  {
    // setup view and proj matrix
    uniform("arealight", "viewMatrix", viewMatrix());
    uniform("arealight", "projMatrix", projectionMatrix());
  }
  glUseProgram(0);

  glUseProgram(shader("ltc_quad"));
  {
    uniform("ltc_quad", "normal", 0);
    uniform("ltc_quad", "position", 1);
    uniform("ltc_quad", "depth", 2);

    uniform("ltc_quad", "roughness", roughness);

    uniform("ltc_quad", "clipless", clipless);

    uniform("ltc_quad", "camera_position", m_cam.position);

    uniform("ltc_quad", "ltc_1", 3);
    uniform("ltc_quad", "ltc_2", 4);

    uniform("ltc_quad", "num_lights", (int)(area_lights.size()));
  }
  glUseProgram(0);
  
  // first draw the lights
  for (unsigned int i = 0; i < area_lights.size(); ++i) {
    AreaLight l = area_lights[i];
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, l.light_position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_y), glm::vec3(0.0,1.0,0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(l.rotation_x), glm::vec3(1.0,0.0,0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(l.scale_x, 1.0, l.scale_y));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    glUseProgram(shader("arealight"));
    {
      // draw the area light
      uniform("arealight", "modelMatrix", modelMatrix);
      uniform("arealight", "u_color", glm::vec3(1.0));
      plane.draw();

      // draw the points passed to the ltc shader
      glPointSize(10.0f);
      for (int j = 0; j < 4; ++j) {
        current_points[j] = modelMatrix * points[j];
        current_points[j] = current_points[j] / current_points[j].a;
        glm::mat4 pModel = glm::translate(glm::mat4(1.0f), glm::vec3(current_points[j]));
        uniform("arealight", "modelMatrix", pModel);
        uniform("arealight", "u_color", colors[j]);
        point.draw();
      }
    }
    glUseProgram(0);

    // draw with ltc
    glUseProgram(shader("ltc_quad"));
    std::stringstream ss;
    ss << "area_lights[" << i << "].";
    uniform("ltc_quad", ss.str() + "intensity", l.light_intensity);
    uniform("ltc_quad", ss.str() + "dcolor", l.diff_color);
    uniform("ltc_quad", ss.str() + "scolor", l.spec_color);

    uniform("ltc_quad", ss.str() + "p1", glm::vec3(current_points[0]));
    uniform("ltc_quad", ss.str() + "p2", glm::vec3(current_points[1]));
    uniform("ltc_quad", ss.str() + "p3", glm::vec3(current_points[2]));
    uniform("ltc_quad", ss.str() + "p4", glm::vec3(current_points[3]));

    // bind all textures
    glActiveTexture(GL_TEXTURE0);
    tex_normal.bind();
    glActiveTexture(GL_TEXTURE1);
    tex_position.bind();
    glActiveTexture(GL_TEXTURE2);
    tex_depth.bind();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ltc_texture_2);
    // draw screen quad
    quad.draw();

    glUseProgram(0);
  }
}

BasicLTC::BasicLTC(std::string const& resource_path)
 :Application{resource_path + "basic_ltc/"}
 ,quad{}
 ,teaPot{m_resource_path + "../shared/data/teapot.obj"}
 ,plane{0.0f, 12.0f}
 ,point{}
 ,ltc_texture_1{0}
 ,ltc_texture_2{0}
 ,gbuffer{}
 ,tex_normal{resolution(), GL_RGBA32F}
 ,tex_position{resolution(), GL_RGBA32F}
 ,tex_depth{resolution(), GL_DEPTH_COMPONENT}
 ,rtt_framebuffer{0}
 ,depthbuffer{0}
 ,rtt_texture{0}
 ,area_lights{
   {
     AreaLight(
         glm::vec3(-5.0, 5.0, 0.0),
         -90.0f,
         90.0f,
         0.8f,
         0.8f,
         5.0f,
         glm::vec3(1.0, 0.2, 1.0),
         glm::vec3(1.0, 0.2, 1.0)),
     AreaLight(
         glm::vec3(5.0, 5.0, 0.0),
         -90.0f,
         -90.0f,
         0.8f,
         0.8f,
         5.0f,
         glm::vec3(1.0, 1.0, 0.0),
         glm::vec3(1.0, 1.0, 0.0)),
     AreaLight(),
     AreaLight(),
     AreaLight(),
     AreaLight(),
     AreaLight(),
     AreaLight(),
     //AreaLight(
     //    glm::vec3(5.0, 5.0, 0.0),
     //    -90.0f,
     //    -90.0f,
     //    0.8f,
     //    0.8f,
     //    5.0f,
     //    glm::vec3(1.0, 1.0, 0.0),
     //    glm::vec3(1.0, 1.0, 0.0))
   }
 }
 ,roughness{0.25f}
 ,clipless{}
{
  initializeGUI();
  initializeObjects();
  initializeShaderPrograms(); 
}

void BasicLTC::initializeGUI() {
  for (unsigned int i = 0; i < area_lights.size(); ++i) {
    AreaLight& l = area_lights[i]; // reference!
    std::stringstream ss;
    ss << i;
    TwAddSeparator(tweakBar, ("sep" + ss.str()).c_str(), nullptr);
    TwAddVarRW(tweakBar, ("light_position" + ss.str()).c_str(), TW_TYPE_DIR3F, &(l.light_position), "label='light_position'");
    TwAddVarRW(tweakBar, ("rotation_x" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.rotation_x), "label='rotation_x' min=0 step=1.0 max=360");
    TwAddVarRW(tweakBar, ("rotation_y" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.rotation_y), "label='rotation_y' min=0 step=1.0 max=360");
    TwAddVarRW(tweakBar, ("scale_x" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.scale_x), "label='scale_x' min=0.1 step=0.1 max=10");
    TwAddVarRW(tweakBar, ("scale_y" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.scale_y), "label='scale_y' min=0.1 step=0.1 max=10");
    TwAddVarRW(tweakBar, ("light_intensity" + ss.str()).c_str(), TW_TYPE_FLOAT, &(l.light_intensity), "label='light_intensity' min=0.1 step=0.1 max=30");
    TwAddVarRW(tweakBar, ("diff_color" + ss.str()).c_str(), TW_TYPE_COLOR3F, &(l.diff_color), "label='diff_color'");
    TwAddVarRW(tweakBar, ("spec_color" + ss.str()).c_str(), TW_TYPE_COLOR3F, &(l.spec_color), "label='spec_color'");
  }

  TwAddSeparator(tweakBar, "sep123", nullptr);
  TwAddVarRW(tweakBar, "roughness", TW_TYPE_FLOAT, &roughness, "label='roughness' min=0.01 step=0.001 max=1");

  TwAddSeparator(tweakBar, "sep124", nullptr);
  TwAddVarRW(tweakBar, "clipless", TW_TYPE_BOOLCPP, &clipless, "label='clipless'");
}

// load shader programs
void BasicLTC::initializeShaderPrograms() {
  initializeShader("ltc_gbuffer",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc_gbuffer.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc_gbuffer.fs.glsl"}});

  initializeShader("arealight",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/arealight.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/arealight.fs.glsl"}});

  initializeShader("ltc_quad",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc_quad.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc_quad.fs.glsl"}});

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

  // re-initialize objects for deffered shading
  gbuffer = Fbo{};
  //tex_diffuse = Tex{resolution(), GL_RGBA32F};
  tex_normal = Tex{resolution(), GL_RGBA32F};
  tex_position = Tex{resolution(), GL_RGBA32F};
  tex_depth = Tex{resolution(), GL_DEPTH_COMPONENT32};
  gbuffer.bind();
  //gbuffer.addTextureAsColorbuffer(tex_diffuse);
  gbuffer.addTextureAsColorbuffer(tex_normal);
  gbuffer.addTextureAsColorbuffer(tex_position);
  gbuffer.addTextureAsDepthbuffer(tex_depth);
  gbuffer.check();
  gbuffer.unbind();

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
