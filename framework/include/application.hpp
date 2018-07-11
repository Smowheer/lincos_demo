#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <map>

#include <glm/gtc/type_precision.hpp>

#include <glbinding/gl/types.h>
// use gl definitions from glbinding 
using namespace gl;

#include "helper.hpp"

class GLFWwindow;

struct CTwBar;
typedef struct CTwBar TwBar;

class Application {
 public:
  // allocate and initialize objects
  Application(std::string const& resource_path);
  Application(Application const&) = delete;
  Application& operator=(Application const&) = delete;
  // free resources
  virtual ~Application();

  // draw all objects
  virtual void render() = 0;

  void updateCamera();
  // handle key input
  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  //handle mouse movement input
  void cursorCallback(GLFWwindow* window, double pos_x, double pos_y);
  //handle mpuse button input 
  void buttonCallback(GLFWwindow* window, int button, int action, int mods);
  // handle scrolling input
  void scrollCallback(GLFWwindow* w, double offset_x, double offset_y);
  // handle resizing
  void resizeCallback(GLFWwindow* window, int width, int height);

 protected:
  // resize callback for derived applications
  virtual void resize();

  void updateShaderPrograms();
  uint32_t shader(std::string const& name) const;
  void initializeShader(std::string const& name, std::map<GLenum, std::string> const& files);

  // get uniform location, throwing exception if name describes no active uniform variable
  GLint glGetUniformLocation(GLuint, const GLchar*) const;

  // upload uniform by name wrapper function
  template<typename T>
  void uniform(std::string const& program, const std::string &name, T const& value) const;
  // uniform upload function
  template<typename T>
  void uniform(int program, const std::string &name, T const& value) const;

  glm::fmat4 const& viewMatrix() const;
  glm::fmat4 const& projectionMatrix() const;
  glm::uvec2 const& resolution() const;
  std::string m_resource_path; 

  // camera
  cameraSystem m_cam;
  // tweak bar
  TwBar* tweakBar;

 private:
  // shader storage
  std::map<std::string, uint32_t> m_shader_handles{};
  std::map<std::string, std::map<GLenum, std::string>> m_shader_files{};
  // mouse buttons
  bool m_pressed_right;
  bool m_pressed_middle;
  bool m_pressed_left;
  // transform matrices
  glm::fmat4 m_viewMatrix;
  glm::fmat4 m_projMatrix;
  glm::uvec2 m_resolution;

};


#include "uniform_upload.hpp"
template<typename T>
void Application::uniform(std::string const& program, const std::string &name, T const& value) const {
  int handle = m_shader_handles.at(program);
  uniform(handle, name, value);
}

template<typename T>
void Application::uniform(int program, const std::string &name, T const& value) const {
  int loc = glGetUniformLocation(program, name.c_str());
  glUniform(loc, value);
}

#endif
