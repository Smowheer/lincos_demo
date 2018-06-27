#pragma once

#include <vector>
#include <string>
#include <chrono>

#include <glbinding/gl/types.h>
// use gl definitions from glbinding 
using namespace gl;

#include <glm/gtc/type_precision.hpp>

const int WIDTH = 1024;
const int HEIGHT = 768;

// Texture
class Tex {
	glm::uvec2 m_dimensions;
	GLuint m_index;
public: 
	Tex(unsigned w, unsigned h, GLenum internal_format);
	Tex(glm::uvec2 const& dims, GLenum internal_format);
	Tex(std::string const& filename);
	Tex(Tex&&);
	Tex(Tex const&) = delete;
	Tex& operator=(Tex&&);
	Tex& operator=(Tex const&) = delete;
	~Tex();
	void bind() const;
	GLuint index() const;
	glm::uvec2 const& dimensions() const;

	friend void swap(Tex& lhs, Tex& rhs);
};
void swap(Tex& lhs, Tex& rhs);

// Frame Buffer Object
class Fbo {
	GLuint id;
	std::vector<GLenum> attachment_ids;
	
public:
	Fbo();
	Fbo(Fbo const&) = delete;
	Fbo(Fbo &&);
	Fbo& operator=(Fbo&&);
	Fbo& operator=(Fbo const&) = delete;
	~Fbo();
	void bind() const;
	void addTextureAsColorbuffer(Tex const& img);
	void addTextureAsDepthbuffer(Tex const& img);
	void unbind() const;
	void check() const;

	friend void swap(Fbo& lhs, Fbo& rhs);
};
void swap(Fbo& lhs, Fbo& rhs);

// timer
class Timer {
 public:
	Timer();
	void update();
	float intervall;
 private:
	std::chrono::system_clock::time_point startTime;
};

// camera stuff
class cameraSystem {
public:
	glm::fvec3 position;   // position-vector
	glm::fvec3 viewDir;    // viewing direction
	glm::fvec3 rightDir;   // right-vector (cross product of viewing- and up-direction)
	glm::fvec3 upDir;      // up-vector

	float delta, mouseDelta;
	int currentX, currentY;

	cameraSystem(float delta, float mouseDelta, glm::vec3 pos);
	cameraSystem(float delta, float mouseDelta, glm::vec3 pos, glm::vec3 dir, glm::vec3 up);

	void moveForward(float delta);
	void moveBackward(float delta);
	void moveUp(float delta);
	void moveDown(float delta);
	void moveRight(float delta);
	void moveLeft(float delta);
	void yaw(float angle);
	void pitch(float angle);
	void roll(float angle);
};
