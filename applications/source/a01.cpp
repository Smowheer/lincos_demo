#include "a01.hpp"

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

void Assignment1::render() {
  
  timer.update();
  if (autoRotate)
    rotationAngle += timer.intervall*degreesPerSecond;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // generate gbuffer
  fbo.bind();
  {
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    renderScene();
  }
  fbo.unbind();

  // shade with primary light source
  {
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader("quad"));

    // TODO A1 (a), setup gbuffer textures: see, glActiveTexture, Tex, uniform (helper.h)
    uniform("quad", "debug", debugShading);

    uniform("quad", "lightDir", lightDir);
    uniform("quad", "viewMatrix", viewMatrix());

    quad.draw();

    if (!debugShading) {
      drawSplats(shader("gbuffer"), 0.1f, false);
    }
  }

  // generate splat stencil buffer
  if (stencilCulling && !debugShading)
  {
    glUseProgram(shader("stencil"));
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    // TODO A1 (b), setup stencil buffer
    drawSplats(shader("stencil"), radius, true);

    // TODO A1 (b), reset to opengl default values

    //stencil debug
    if (debugStencil) {
      if (clearDebugStencil) {
        glClearColor(0.f, 0.f, 0.f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }

      // TODO A1 (b), setup depth and stencil setting for masking
      glUseProgram(shader("stencilDebug"));
      quad.draw();

      glDepthFunc(GL_LESS);
    }
  }

  // actual splatting
  // TODO A1 (b) & (c), take care of correct culling modes
  if (!debugStencil && splatting && !debugShading)  
  {
    glDepthMask(false);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_ALWAYS);
    if (stencilCulling) {
      glStencilFunc(GL_EQUAL, 1, ~0);
      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

    glUseProgram(shader("splat"));
    uniform("splat", "resolution", resolution());

    uniform("splat", "diffuse", 0);
    uniform("splat", "normal", 1);
    uniform("splat", "position", 2);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    drawSplats(shader("splat"), radius, true);

    glDisable(GL_BLEND);
    glDepthMask(true);
    glDisable(GL_CULL_FACE);
  }

  glDisable(GL_STENCIL_TEST);
  glDepthFunc(GL_LESS);

  glUseProgram(0);
}

void Assignment1::renderScene() const {

  glUseProgram(shader("gbuffer"));
  
  uniform("gbuffer", "color", glm::fvec3(0.8f, 0.8f, 0.8f));
  uniform("gbuffer", "viewMatrix", viewMatrix());
  uniform("gbuffer", "projMatrix", projectionMatrix());

  // render pots
  for (int i = 0; i < 10; ++i) {
    float rad = ((float)i/10.f) * (float)M_PI * 2;
    float dist = 8.f;
    float x = glm::cos(rad)*dist;
    float y = glm::sin(rad)*dist;
    uniform("gbuffer", "modelMatrix", glm::translate(glm::fvec3(x, 0, y)) * glm::scale(glm::fmat4(), glm::fvec3(2.5, 2.5, 2.5)));
    teaPot.draw();
  }

  // render ground plane
  uniform("gbuffer", "modelMatrix", glm::fmat4());
  plane.draw();

  glUseProgram(0);
}

float Assignment1::splatDistFromOrigin() const {
  return (glm::cos(rotationAngle*0.7f) + 1.0f) * 4.f + 1.0f;
}

void Assignment1::drawSplats(const int program, const float radius, const bool asLight) const {

  glUseProgram(program);
  uniform(program, "viewMatrix", viewMatrix());
  uniform(program, "projMatrix", projectionMatrix());


  glm::fvec3 splatColors[6] = { glm::fvec3(1, 0, 0),
                 glm::fvec3(1, 1, 0),
                 glm::fvec3(0, 1, 0),
                 glm::fvec3(0, 1, 1),
                 glm::fvec3(0, 0, 1),
                 glm::fvec3(1, 0, 1) };

  for (int i = 0; i < 6; ++i) {
    float rad = ((float)i/6.f) * (float)M_PI * 2 + rotationAngle;
    float dist = splatDistFromOrigin();
    float x = glm::cos(rad)*dist;
    float y = glm::sin(rad)*dist;
    if (asLight) {
      uniform(program, "radius", radius);
      uniform(program, "splatColor", splatColors[i]); 
    }
    else 
      uniform(program, "color", splatColors[i]);
    uniform(program, "modelMatrix", glm::translate(glm::fvec3(x, 2.8, y)) * glm::scale(glm::fmat4(), glm::fvec3(radius, radius, radius)));
    sphere.draw();
  }

  glUseProgram(0);
}

Assignment1::Assignment1(std::string const& resource_path)
 :Application{resource_path + "a01/"}
 ,quad{}
 ,teaPot{m_resource_path + "../shared/data/teapot.obj"}
 ,plane{0.f, 12.f}
 ,sphere{1, 20, 20}
 ,point{}
 ,fbo{}
 ,diffuse{resolution(), GL_RGBA32F}
 ,normal{resolution(), GL_RGBA32F}
 ,position{resolution(), GL_RGBA32F}
 ,depth{resolution(), GL_DEPTH_COMPONENT32}
 ,timer{}
 ,lightDir{glm::normalize(glm::fvec3(0.f, 0.f, 1.f))}
 ,rotationAngle{0.0f}
 ,degreesPerSecond{1.f}
 ,radius{8.0f}
 ,autoRotate{true}
 ,debugShading{false}
 ,stencilCulling{false}
 ,debugStencil{false}
 ,clearDebugStencil{false}
 ,splatting{false}
{
  initializeGUI();
  initializeObjects();
  initializeShaderPrograms(); 
}

void Assignment1::initializeGUI() {
  TwAddVarRW(tweakBar, "lightDirection", TW_TYPE_DIR3F, &lightDir, "label='Light Direction'");
  TwAddVarRW(tweakBar, "Rotate", TW_TYPE_BOOLCPP, &autoRotate, " label='Auto Rotation' ");
  TwAddVarRW(tweakBar, "RotSpeed", TW_TYPE_FLOAT, &degreesPerSecond, " label='Rotation Speed' min=0 step=0.1 max=360 ");

  TwAddSeparator(tweakBar, "sep0", nullptr);
  TwAddVarRW(tweakBar, "DebugShader", TW_TYPE_BOOLCPP, &debugShading, " label='Debug Shader' ");

  TwAddSeparator(tweakBar, "sep1", nullptr);
  TwAddVarRW(tweakBar, "StencilCulling", TW_TYPE_BOOLCPP, &stencilCulling, " label='Stencil Culling' ");
  TwAddVarRW(tweakBar, "StencilDebug", TW_TYPE_BOOLCPP, &debugStencil, " label='Debug' ");
  TwAddVarRW(tweakBar, "StencilDebugClear", TW_TYPE_BOOLCPP, &clearDebugStencil, " label='Clear' ");

  TwAddSeparator(tweakBar, "sep2", nullptr);
  TwAddVarRW(tweakBar, "Splatting", TW_TYPE_BOOLCPP, &splatting, " label='Splatting' ");
  TwAddVarRW(tweakBar, "SplatSize", TW_TYPE_FLOAT, &radius, " label='Splat Size' min=1 max=10 step=0.5");
}

// load shader programs
void Assignment1::initializeShaderPrograms() {
  initializeShader("quad",{{GL_VERTEX_SHADER, m_resource_path + "./shader/quad.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/quad.fs.glsl"}});
  initializeShader("stencil",{{GL_VERTEX_SHADER, m_resource_path + "./shader/stencil.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/stencil.fs.glsl"}});
  initializeShader("stencilDebug",{{GL_VERTEX_SHADER, m_resource_path + "./shader/debug.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/debug.fs.glsl"}});
  initializeShader("splat",{{GL_VERTEX_SHADER, m_resource_path + "./shader/splat.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/splat.fs.glsl"}});
  initializeShader("gbuffer",{{GL_VERTEX_SHADER, m_resource_path + "./shader/gbuffer.vs.glsl"}, {GL_FRAGMENT_SHADER, m_resource_path + "./shader/gbuffer.fs.glsl"}});
}

void Assignment1::initializeObjects() {
  fbo.bind();
  fbo.addTextureAsColorbuffer(diffuse);
  fbo.addTextureAsColorbuffer(normal);
  fbo.addTextureAsColorbuffer(position);
  fbo.addTextureAsDepthbuffer(depth);
  fbo.check();
  fbo.unbind();
}

void Assignment1::resize() {
  fbo = Fbo{};
  diffuse = Tex{resolution(), GL_RGBA32F};
  normal = Tex{resolution(), GL_RGBA32F};
  position = Tex{resolution(), GL_RGBA32F};
  depth = Tex{resolution(), GL_DEPTH_COMPONENT32};
  initializeObjects();
}

#include "launcher.hpp"
// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<Assignment1>(argc, argv);
}
