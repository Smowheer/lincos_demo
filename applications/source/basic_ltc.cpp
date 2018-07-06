
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
    render_ltc_quad();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // TODO: how to render light sources?
  // two ideas:
  // 1. Render light source slightly behind (planar polygonal light sources)
  // 2. Render somehow after some stage TODO

  // draw final with ACES
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shader("ltc_blit"));
  uniform("ltc_blit", "resolution", glm::vec2(resolution()));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rtt_texture);
  uniform("ltc_blit", "tex", 0);
  quad.draw();
  glUseProgram(0);
  
  // old version
  //glBindFramebuffer(GL_FRAMEBUFFER, rtt_framebuffer);
  //glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //renderScene();
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);


  //glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glUseProgram(shader("blit"));
  //uniform("blit", "resolution", glm::vec2(resolution()));
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, rtt_texture);
  //uniform("blit", "tex", 0);
  //quad.draw();
  //glUseProgram(0);
}

void BasicLTC::render_gbuffer() {
  // draw the ground
  glUseProgram(shader("ltc_gbuffer"));

  // vertex shader uniforms
  uniform("ltc_gbuffer", "modelMatrix", glm::mat4(1.0));
  uniform("ltc_gbuffer", "viewMatrix", viewMatrix());
  uniform("ltc_gbuffer", "projMatrix", projectionMatrix());
  plane.draw();

  // draw the pot ( uniforms stay the same :) )
  teaPot.draw();

  glUseProgram(0);
}

void BasicLTC::render_ltc_quad() {
  glm::mat4 modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0,3.0,-5.0));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

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
  for (int i = 0; i < 4; ++i) {
    points[i] = modelMatrix * points[i];
    points[i] = points[i] / points[i].a;
    glm::mat4 pModel = glm::translate(glm::mat4(1.0f), glm::vec3(points[i]));
    uniform("arealight", "modelMatrix", pModel);
    uniform("arealight", "u_color", colors[i]);
    point.draw();
  }

  // draw the ground
  glUseProgram(shader("ltc_quad"));

  // fragment shader uniforms
  uniform("ltc_quad", "normal", 0);
  uniform("ltc_quad", "position", 1);
  uniform("ltc_quad", "depth", 2);

  uniform("ltc_quad", "intensity", light_intensity);
  uniform("ltc_quad", "dcolor", diff_color);
  uniform("ltc_quad", "scolor", spec_color);

  uniform("ltc_quad", "roughness", roughness);

  uniform("ltc_quad", "clipless", clipless);

  uniform("ltc_quad", "p1", glm::vec3(points[0]));
  uniform("ltc_quad", "p2", glm::vec3(points[1]));
  uniform("ltc_quad", "p3", glm::vec3(points[2]));
  uniform("ltc_quad", "p4", glm::vec3(points[3]));

  uniform("ltc_quad", "camera_position", m_cam.position);

  uniform("ltc_quad", "ltc_1", 3);
  uniform("ltc_quad", "ltc_2", 4);

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

  quad.draw();

  glUseProgram(0);
}

void BasicLTC::renderScene() {

  glm::mat4 modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0,3.0,-5.0));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0));
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

  // draw the ground
  glUseProgram(shader("ltc"));

  // vertex shader uniforms
  uniform("ltc", "modelMatrix", glm::mat4(1.0));
  uniform("ltc", "viewMatrix", viewMatrix());
  uniform("ltc", "projMatrix", projectionMatrix());

  // fragment shader uniforms
  uniform("ltc", "intensity", light_intensity);
  uniform("ltc", "dcolor", diff_color);
  uniform("ltc", "scolor", spec_color);

  uniform("ltc", "roughness", roughness);

  uniform("ltc", "clipless", clipless);

  uniform("ltc", "p1", glm::vec3(points[0]));
  uniform("ltc", "p2", glm::vec3(points[1]));
  uniform("ltc", "p3", glm::vec3(points[2]));
  uniform("ltc", "p4", glm::vec3(points[3]));

  uniform("ltc", "camera_position", m_cam.position);

  uniform("ltc", "ltc_1", 0);
  uniform("ltc", "ltc_2", 1);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ltc_texture_2);

  plane.draw();

  //uniform("ltc", "modelMatrix", glm::translate(glm::mat4(1.0), glm::vec3(0.0, 4.0, 0.0)));
  teaPot.draw();

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
 ,gbuffer{}
 //,tex_diffuse{resolution(), GL_RGBA32F}
 ,tex_normal{resolution(), GL_RGBA32F}
 ,tex_position{resolution(), GL_RGBA32F}
 ,tex_depth{resolution(), GL_DEPTH_COMPONENT}
 ,rtt_framebuffer{0}
 ,depthbuffer{0}
 ,rtt_texture{0}
 ,light_position{glm::vec3(0.0)}
 ,rotation_x{0.0f}
 ,rotation_y{0.0f}
 ,scale_x{1.0f}
 ,scale_y{1.0f}
 ,light_intensity{15.0f}
 ,diff_color{glm::vec3(0.0)}
 ,spec_color{glm::vec3(1.0)}
 ,roughness{0.55f}
 ,clipless{}
{
  initializeGUI();
  initializeObjects();
  initializeShaderPrograms(); 
}

void BasicLTC::initializeGUI() {
  TwAddVarRW(tweakBar, "light_position", TW_TYPE_DIR3F, &light_position, "label='light_position'");
  TwAddVarRW(tweakBar, "rotation_x", TW_TYPE_FLOAT, &rotation_x, "label='rotation_x' min=0 step=1.0 max=360");
  TwAddVarRW(tweakBar, "rotation_y", TW_TYPE_FLOAT, &rotation_y, "label='rotation_y' min=0 step=1.0 max=360");
  TwAddVarRW(tweakBar, "scale_x", TW_TYPE_FLOAT, &scale_x, "label='scale_x' min=0.1 step=0.1 max=10");
  TwAddVarRW(tweakBar, "scale_y", TW_TYPE_FLOAT, &scale_y, "label='scale_y' min=0.1 step=0.1 max=10");
  TwAddVarRW(tweakBar, "light_intensity", TW_TYPE_FLOAT, &light_intensity, "label='light_intensity' min=0.1 step=0.1 max=20");
  TwAddVarRW(tweakBar, "diff_color", TW_TYPE_COLOR3F, &diff_color, "label='diff_color'");
  TwAddVarRW(tweakBar, "spec_color", TW_TYPE_COLOR3F, &spec_color, "label='spec_color'");

  TwAddSeparator(tweakBar, "sep0", nullptr);
  TwAddVarRW(tweakBar, "roughness", TW_TYPE_FLOAT, &roughness, "label='roughness' min=0.01 step=0.001 max=1");

  TwAddSeparator(tweakBar, "sep1", nullptr);
  TwAddVarRW(tweakBar, "clipless", TW_TYPE_BOOLCPP, &clipless, "label='clipless'");
}

// load shader programs
void BasicLTC::initializeShaderPrograms() {
  initializeShader("ltc",{{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc2.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc2.fs.glsl"}});
  initializeShader("arealight",{{GL_VERTEX_SHADER, m_resource_path + "./shader/arealight.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/arealight.fs.glsl"}});
  initializeShader("blit",{{GL_VERTEX_SHADER, m_resource_path + "./shader/blit_ltc.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/blit_ltc.fs.glsl"}});

  initializeShader("ltc_gbuffer",
      {{GL_VERTEX_SHADER, m_resource_path + "./shader/ltc_gbuffer.vs.glsl"},
      {GL_FRAGMENT_SHADER, m_resource_path + "./shader/ltc_gbuffer.fs.glsl"}});

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
