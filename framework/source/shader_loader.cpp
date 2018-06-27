#include "shader_loader.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <glbinding/gl/functions.h>
// use gl definitions from glbinding 
using namespace gl;

// hidden helper functions, moved from utils
namespace {

std::string file_name(std::string const& file_path) {
  return file_path.substr(file_path.find_last_of("/\\") + 1);
}

void output_log(GLchar const* log_buffer, std::string const& header) {
  std::cerr << header << ":\n";
  std::string error{};
  std::istringstream error_stream{log_buffer};
  while(std::getline(error_stream, error)) {
    std::cerr << " " << error << std::endl;
  }
}

std::string read_file(std::string const& name) {
  std::ifstream ifile(name);

  if(ifile) {
    std::string filetext;
    
    while(ifile.good()) {
      std::string line;
      std::getline(ifile, line);
      filetext.append(line + "\n");
    }
    
    return filetext; 
  }
  else {
    std::cerr << "File \'" << name << "\' not found" << std::endl;
    
    throw std::invalid_argument(name);
  }
}
}

namespace shader_loader {

GLuint shader(std::string const& file_path, GLenum shader_type) {
  GLuint shader = 0;
  shader = glCreateShader(shader_type);

  std::string shader_source{read_file(file_path)};
  // glshadersource expects array of c-strings
  const char* shader_chars = shader_source.c_str();
  glShaderSource(shader, 1, &shader_chars, 0);

  glCompileShader(shader);

  // check if compilation was successfull
  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(success == 0) {
    // get log length
    GLint log_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    // get log
    GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
    glGetShaderInfoLog(shader, log_size, &log_size, log_buffer);
    // output errors
    output_log(log_buffer, file_name(file_path));
    // free broken shader
    glDeleteShader(shader);
    free(log_buffer);

    throw std::logic_error("OpenGL error: compilation of " + file_path);
  }

  return shader;
}

unsigned program(std::map<GLenum, std::string> const& stages) {
  unsigned program = glCreateProgram();

  std::vector<GLuint> shaders{};
  // load and compile vert and frag shader
  for (auto const& stage : stages) {
    GLuint shader_handle = shader(stage.second, stage.first);
    shaders.push_back(shader_handle);
    // attach the shader to program
    glAttachShader(program, shader_handle);
  }

  // link shaders
  glLinkProgram(program);

  // check if linking was successfull
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if(success == 0) {
    // get log length
    GLint log_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
    // get log
    GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
    glGetProgramInfoLog(program, log_size, &log_size, log_buffer);
    
    // output errors
    std::string paths{};
    for(auto const& stage : stages) {
      paths += file_name(stage.second) + " & ";
    }
    paths.resize(paths.size() - 3);
    output_log(log_buffer, paths);
    // free broken program
    glDeleteProgram(program);
    free(log_buffer);

    throw std::logic_error("OpenGL error: linking of " + paths);
  }

  for (auto shader_handle : shaders) {
    // detach shader
    glDetachShader(program, shader_handle);
    // and free it
    glDeleteShader(shader_handle);
  }

  return program;
}

}
