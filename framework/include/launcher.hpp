#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <string>

#include "application.hpp"

// forward declarations
class Application;
class GLFWwindow;

//glut stuff
void keyboard(GLFWwindow* w, int key, int scancode, int action, int mods);
void onMouseMove(GLFWwindow* w, double x, double y);
void onMouseDown(GLFWwindow* w, int button, int action, int mods);

class Launcher {
 public:
  template<typename T>
  static void run(int argc, char* argv[]) {
    Launcher launcher{argc, argv};
    launcher.run<T>();
  }

 private:

  Launcher(int argc, char* argv[]);
  Launcher(Launcher const&) = delete;
  Launcher& operator=(Launcher const&) = delete;

  // run application
  template<typename T>
  void run(){
    initialize();

    m_application = new T{m_resource_path};

    mainLoop();
  }
  
  // create window and set callbacks
  void initialize();
  // start main loop
  void mainLoop();
  // free resources
  void quit(int status);

  // the rendering window
  GLFWwindow* m_window;

  // path to the resource folders
  std::string m_resource_path;

  Application* m_application;

};
#endif