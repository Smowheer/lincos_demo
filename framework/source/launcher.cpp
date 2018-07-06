#include "launcher.hpp"

#include <functional>
#include <iostream>

#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>
// load meta info extension
#include <glbinding/Meta.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <AntTweakBar.h>

#include "application.hpp"

// helper functions
std::string resourcePath(int argc, char* argv[]);
void glfw_error(int error, const char* description);
void TW_CALL atb_error(const char* description);
void watch_gl_errors(bool activate = true);

Launcher::Launcher(int argc, char* argv[])
 :m_window{}
 ,m_resource_path{resourcePath(argc, argv)}
 ,m_application{}
{}

std::string resourcePath(int argc, char* argv[]) {
  std::string resource_path{};
  //first argument is resource path
  if (argc > 1) {
    resource_path = argv[1];
  }
  // no resource path specified, use default
  else {
    std::string exe_path{argv[0]};
    resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
    resource_path += "/resources/";
  }

  return resource_path;
}

static void APIENTRY openglCallbackFunction(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
){
  (void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || type == GL_DEBUG_TYPE_PERFORMANCE) {
    return;
  }
  std::cerr << glbinding::Meta::getString(severity) << " - " << glbinding::Meta::getString(type) << ": ";
  std::cerr << message << std::endl;
  // if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && type != GL_DEBUG_TYPE_PERFORMANCE) {
  //   throw std::runtime_error{"OpenGL error"};
  // }
}

void Launcher::initialize() {

  glfwSetErrorCallback(glfw_error);

  if (!glfwInit()) {
    std::exit(EXIT_FAILURE);
  }

  // set OGL version explicitly 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create m_window, if unsuccessfull, quit
  m_window = glfwCreateWindow(WIDTH, HEIGHT, "LinCos Demo", NULL, NULL);
  if (!m_window) {
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // use the windows context
  glfwMakeContextCurrent(m_window);
  // enable vsync
  glfwSwapInterval(1);

  // initialize glindings in this context
  glbinding::Binding::initialize();

  // activate error checking after each gl function call
  watch_gl_errors();

  // Enable the debug callback
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openglCallbackFunction, nullptr);
  glDebugMessageControl(
    GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true
  );

  // initalize AntTweakBar
  TwInit(TW_OPENGL_CORE, NULL);
  TwHandleErrors(atb_error);
}
 
void Launcher::mainLoop() {

  // set user pointer to access this instance statically
  glfwSetWindowUserPointer(m_window, m_application);
  // register key input function
  auto key_func = [](GLFWwindow* w, int a, int b, int c, int d) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->keyCallback(w, a, b, c, d);
  };
  glfwSetKeyCallback(m_window, key_func);
  // register mouse movement function
  auto cursor_func = [](GLFWwindow* w, double a, double b) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->cursorCallback(w, a, b);
  };
  glfwSetCursorPosCallback(m_window, cursor_func);
  // register mouse buttonm function
  auto button_func = [](GLFWwindow* w, int a, int b, int c) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->buttonCallback(w, a, b, c);
  };
  glfwSetMouseButtonCallback(m_window, button_func);
  // register mouse scroll function
  auto scroll_func = [](GLFWwindow* w, double a, double b) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->scrollCallback(w, a, b);
  };
  glfwSetScrollCallback(m_window, scroll_func);
  // register resizing function
  auto resize_func = [](GLFWwindow* w, int a, int b) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->resizeCallback(w, a, b);
  };
  glfwSetFramebufferSizeCallback(m_window, resize_func);

  double lastTime = glfwGetTime();
  int nbFrames = 0;
  // rendering loop
  while (!glfwWindowShouldClose(m_window)) {
    // Measure speed
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
      // printf and reset timer
      printf("%f ms/frame\n", 1000.0/double(nbFrames));
      nbFrames = 0;
      lastTime += 1.0;
    }

    // query input
    glfwPollEvents();
    // render scene
    m_application->render();
    // tweak bar
    TwDraw();
    // swap draw buffer to front
    glfwSwapBuffers(m_window);
  }

  quit(EXIT_SUCCESS);
}


void Launcher::quit(int status) {
  // Terminate AntTweakBar
  TwTerminate();
  // free opengl resources
  delete m_application;

  // free glfw resources
  glfwDestroyWindow(m_window);
  glfwTerminate();

  std::exit(status);
}


void glfw_error(int error, const char* description) {
  std::cerr << "GLFW Error " << error << " : " << description << std::endl;
  // throw exception to allow for backtrace
  throw std::runtime_error("GLFW error");
  exit(EXIT_FAILURE);
}


void TW_CALL atb_error(const char* description) {
  std::cerr << "AntTweakBar Error: " << description << std::endl;
  // throw exception to allow for backtrace
  throw std::runtime_error("AntTweakBar error");
  exit(EXIT_FAILURE);
}

void watch_gl_errors(bool activate) {
  if(activate) {
    // add callback after each function call
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, {"glGetError", "glBegin", "glVertex3f", "glColor3f"});
    glbinding::setAfterCallback(
      [](glbinding::FunctionCall const& call) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          // print name
          std::cerr <<  "OpenGL Error: " << call.function->name() << "(";
          // parameters
          for (unsigned i = 0; i < call.parameters.size(); ++i)
          {
            std::cerr << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
              std::cerr << ", ";
          }
          std::cerr << ")";
          // return value
          if(call.returnValue) {
            std::cerr << " -> " << call.returnValue->asString();
          }
          // error
          std::cerr  << " - " << glbinding::Meta::getString(error) << std::endl;
          // throw exception to allow for backtrace
          throw std::runtime_error("OpenGL error: " + std::string(call.function->name()));
          exit(EXIT_FAILURE);
        }
      }
    );
  }
  else {
    glbinding::setCallbackMask(glbinding::CallbackMask::None);
  }
}
