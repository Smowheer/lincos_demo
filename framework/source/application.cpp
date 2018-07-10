#include "application.hpp"

#include <iostream>

#include <AntTweakBar.h>

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "shader_loader.hpp"


Application::Application(std::string const& resource_path)
 :m_resource_path{resource_path}
 ,m_cam{1.0f, 1.0f, glm::vec3(0.0f, 5.5f, -10.0f)}
 ,tweakBar{TwNewBar("Settings")}
 ,rendered_frames{0}
 ,m_shader_handles{}
 ,m_shader_files{}
 ,m_pressed_right{false}
 ,m_pressed_middle{false}
 ,m_pressed_left{false}
 ,m_viewMatrix{}
 ,m_projMatrix{}
 ,m_resolution{WIDTH, HEIGHT}
{
  glClearDepth(1);
  glClearColor(0.1f, 0.4f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST); // turn on the depth test

  // initialize view and projection matrices
  updateCamera();
  resizeCallback(nullptr, m_resolution.x, m_resolution.y);
}

Application::~Application() {
  // free all shader program objects
  for (auto const& pair : m_shader_handles) {
    glDeleteProgram(pair.second);
  }
}

void Application::updateShaderPrograms() {
  for (auto const& pair : m_shader_files) {
    auto& handle = m_shader_handles[pair.first];
  	try {
  		auto handle_new = shader_loader::program(pair.second);
    	// if compilation throws exception, old handle is not overridden
    	glDeleteProgram(handle);
    	handle = handle_new;
  	}
  	catch(std::exception&) {}
	}	
}

uint32_t Application::shader(std::string const& name) const {
  return m_shader_handles.at(name);
}

void Application::initializeShader(std::string const& name, std::map<GLenum, std::string> const& files) {
  m_shader_files.emplace(name, files);
  m_shader_handles.emplace(name, shader_loader::program(files));
}

void Application::updateCamera() {
  const glm::vec3 eye =    glm::vec3(m_cam.position);
  const glm::vec3 center = glm::vec3(m_cam.position + m_cam.viewDir);
  // always keep global up vector
  const glm::vec3 up =     glm::vec3(m_cam.upDir);

  m_viewMatrix = glm::lookAt(eye,center,up);
}

void Application::resizeCallback(GLFWwindow* win, int w, int h) {
  m_resolution = glm::uvec2{w, h};
  (void)win;

  glViewport(0,0,(GLsizei)w, (GLsizei)h);

  static float aspect_orig = float(WIDTH) / float(HEIGHT);
  float aspect = float(m_resolution.x) / float(m_resolution.y);
  float fov_y = 70.0f;
  if (aspect < aspect_orig) {
    fov_y = 2.0f * glm::atan(glm::tan(70.0f * 0.5f) * (aspect_orig / aspect));
  }
  // projection is hor+ 
  m_projMatrix = glm::perspective(fov_y, aspect, 1.0f, 100.0f);
  resize();

  TwWindowSize(w,h);
}

void Application::resize() {
  // called during resize callback within constructor
}

void Application::keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
  (void)scancode;
  (void)mods;
  if (TwEventKeyGLFW(key, action)) {
    return;
  }

  switch(key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(w, true);
      break;
    case GLFW_KEY_W:
      m_cam.moveForward(m_cam.delta);
      updateCamera();
      break;
    case GLFW_KEY_S:
      m_cam.moveBackward(m_cam.delta);
      updateCamera();
      break;
    case GLFW_KEY_A:
      m_cam.moveLeft(m_cam.delta);
      updateCamera();
      break;
    case GLFW_KEY_D:
      m_cam.moveRight(m_cam.delta);
      updateCamera();
      break;
    case GLFW_KEY_Q:
      // m_cam.roll(0.01f*m_cam.mouseDelta);
      updateCamera();
      break;
    case GLFW_KEY_E:
      // m_cam.roll(0.01f*-m_cam.mouseDelta);
      updateCamera();
      break;
    case GLFW_KEY_R:
      updateShaderPrograms();
      break;
    case GLFW_KEY_C:
      const glm::vec3& d = m_cam.viewDir;
      const glm::vec3& u = m_cam.upDir;
      const glm::vec3& p = m_cam.position;
      std::cout << "m_cam-dir: " << d.x << ", " << d.y << ", " << d.z << std::endl <<
                   "m_cam-up:  " << u.x << ", " << u.y << ", " << u.z << std::endl <<
             "m_cam-pos: " << p.y << ", " << p.y << ", " << p.z << std::endl;
      break;
  }

  rendered_frames = 0;
}

void Application::buttonCallback(GLFWwindow* w, int button, int action, int mods) {
  (void)w;
  (void)mods;
  if (TwEventMouseButtonGLFW(button, action)) {
    return;
  }

  switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      if (action == GLFW_PRESS)
        m_pressed_left = true;
      else
        m_pressed_left = false;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_PRESS)
        m_pressed_right = true;
      else
        m_pressed_right = false;
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      if (action == GLFW_PRESS)
        m_pressed_middle = true;
      else
        m_pressed_middle = false;
      break;
  }

  rendered_frames = 0;
}

void Application::cursorCallback(GLFWwindow* w, double x, double y) {
  (void)w;
  if (TwEventMousePosGLFW((int)x, (int)y)) {
    return;
  }

  const float dx = float(m_cam.currentX-x);
  const float dy = float(m_cam.currentY-y);

  m_cam.currentX = (int)x;
  m_cam.currentY = (int)y;

  if (m_pressed_right) {
    m_cam.moveUp(-dy*0.03f);
    m_cam.moveRight(dx*0.03f);
  }
  else if (m_pressed_middle) {
    m_cam.moveForward(dy*0.1f*m_cam.mouseDelta);
    // m_cam.roll(dx*0.001f*m_cam.mouseDelta);
  }
  else if (m_pressed_left){
    m_cam.yaw(-dx*0.001f*m_cam.mouseDelta);
    m_cam.pitch(-dy*0.001f*m_cam.mouseDelta);
  }
  // no button pressed -> dont update view matrix
  else {
    return;
  }

  updateCamera();

  rendered_frames = 0;
}

void Application::scrollCallback(GLFWwindow* w, double offset_x, double offset_y) {
  (void)w;
  (void)offset_x;
  if (TwEventMouseWheelGLFW((int)offset_y)) {
    return;
  }

  if (offset_y > 0.0) {
    m_cam.delta *= (4.f / 3.f);
  }
  else {
    m_cam.delta *= 0.75f;
  }
  
  updateCamera();
}

glm::fmat4 const& Application::viewMatrix() const {
  return m_viewMatrix;
}
glm::fmat4 const& Application::projectionMatrix() const {
  return m_projMatrix;
}

glm::uvec2 const& Application::resolution() const {
  return m_resolution;
}



GLint Application::glGetUniformLocation(GLuint program, const GLchar* name) const {
  // use function from outer namespace to prevent recursion
  GLint loc = ::glGetUniformLocation(program, name);
  return loc;
  // bool check = false;
  // // if location invalid, output info similar to gl errors
  // if (check && loc == -1) {
  //   std::cerr <<  "OpenGL Error: " << "glGetUniformLocation" << "(";
  //   std::cerr << program << ", " << name << ") - ";
  //   // if no program is bound
  //   if (program == 0) {
  //     std::cerr << "no program bound" << std::endl;
  //     return loc;
  //   }
  //   // a program is bound
  //   std::cerr << name <<" is not an active uniform variable in program ";
  //   for (auto const& pair : m_shader_handles) {
  //     if (pair.second == program) {
  //       std::cerr << '\'' << pair.first << '\'' << std::endl;
  //     }
  //   }
  //   // dont throw, allow retrying
  //   // throw std::runtime_error("Execution of " + std::string("glGetUniformLocation"));
  //   // exit(EXIT_FAILURE);
  // }
  // return loc;
}
