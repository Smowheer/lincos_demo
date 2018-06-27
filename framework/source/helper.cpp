#include "helper.hpp"

#include <iostream>
#include <cstring>

#include <glm/gtx/transform.hpp>

#include <lodepng.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
// load meta info extension
#include <glbinding/Meta.h>

static GLenum pixel_format(GLenum internal_format);
// Texture
Tex::Tex(unsigned w, unsigned h, GLenum internal_format) 
 :Tex{glm::uvec2{w, h}, internal_format}
{}

Tex::Tex(glm::uvec2 const& dims, GLenum internal_format)
	:m_dimensions{dims}
	,m_index{0}
{
	glGenTextures(1,&m_index);
	glBindTexture(GL_TEXTURE_2D,m_index);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_dimensions.x, m_dimensions.y, 0, pixel_format(internal_format), GL_UNSIGNED_BYTE, nullptr);
}

#include <sstream>
std::string colorTypeString(LodePNGColorType type)
{
  std::string name;
  switch(type)
  {
    case LCT_GREY: name = "grey"; break;
    case LCT_RGB: name = "rgb"; break;
    case LCT_PALETTE: name = "palette"; break;
    case LCT_GREY_ALPHA: name = "grey with alpha"; break;
    case LCT_RGBA: name = "rgba"; break;
    default: name = "invalid"; break;
  }
  std::stringstream ss;
  ss << type << " (" << name << ")";
  return ss.str();
}
Tex::Tex(std::string const& filename)
 :m_dimensions{}
 ,m_index{0}
{
  lodepng::State state;
  std::vector<uint8_t> png;
   //load the image file with given filename
  unsigned error = lodepng::load_file(png, filename);
	if(error) {
		std::cerr << "LodePNG error - " << error << ": " << lodepng_error_text(error) << std::endl;
		std::cerr << "failed to load file " << filename << std::endl;
  	throw std::runtime_error("LodePNG error");
	}
	unsigned width;
	unsigned height;
  std::vector<uint8_t> pixels; //the raw pixels
  lodepng::decode(pixels, width, height, state, png);
	if(error) {
		std::cerr << "LodePNG error - " << error << ": " << lodepng_error_text(error) << std::endl;
  	throw std::runtime_error("LodePNG error");
	}
  // std::cout << "Color: " << colorTypeString(state.info_png.color.colortype) << ", " << state.info_png.color.bitdepth << " bit" << std::endl;
	m_dimensions = glm::uvec2{width, height};
	
	// flip pixels for ogl lookup
	unsigned num_channels = 4;
	std::vector<uint8_t> pixels_flipped(width * height * num_channels, 0);
	for(unsigned y = 0; y < height; ++y) {
		std::memcpy(pixels_flipped.data() + y * width * num_channels, pixels.data() + (height - y - 1)* width * num_channels, sizeof(uint8_t) * num_channels * width);
	}

	glGenTextures(1, &m_index);
	glBindTexture(GL_TEXTURE_2D, m_index);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_flipped.data());
	glGenerateMipmap(GL_TEXTURE_2D);
}

Tex::Tex(Tex&& rhs)
 :m_dimensions{}
 ,m_index{0}
{
	swap(*this, rhs);
}

Tex& Tex::operator=(Tex&& rhs) {
	swap(*this, rhs);
	return *this;
}

void swap(Tex& lhs, Tex& rhs) {
	std::swap(lhs.m_dimensions, rhs.m_dimensions);
	std::swap(lhs.m_index, rhs.m_index);
}

Tex::~Tex() {
	// emplace_back will call move constructor, creating ungenerated texture
	if (m_index != 0) {
		glDeleteTextures(1, &m_index);
	}
}

void Tex::bind() const {
	glBindTexture(GL_TEXTURE_2D, m_index);
}

glm::uvec2 const& Tex::dimensions() const {
	return m_dimensions;
}
GLuint Tex::index() const {
 return m_index; 
}

// helper function to calculate appropriate pixel data format form internal format
static GLenum pixel_format(GLenum internal_format) {
	std::string str_format = glbinding::Meta::getString(internal_format);
	// depth texture
	if (str_format.at(3) == 'D') {
		// pure depth
		if(str_format.at(9) == 'C') {
			return GL_DEPTH_COMPONENT;
		}
		// depth + stencil
		else {
			return GL_DEPTH_STENCIL;
		}
	}
	else if (str_format.at(3) == 'R') {
		if (str_format.at(4) == 'G') {
			if (str_format.at(4) == 'B') {
				if (str_format.at(5) == 'A') {
					return GL_RGBA;
				}
				else {
					return GL_RGB;
				}
			}
			else {
				return GL_RG;
			}
		}
		else {
			return GL_RED;
		}
	}
	else {
		throw std::runtime_error{" internal format '" + str_format + "' not supported"};
	}
}

// Frame Buffer Object
Fbo::Fbo() 
 :id{0}
 ,attachment_ids{}
{
	glGenFramebuffers(1, &id);
}

Fbo::Fbo(Fbo&& rhs)
 :Fbo{}
{
	swap(*this, rhs);
}

Fbo::~Fbo() {
	glDeleteFramebuffers(1, &id);
}

Fbo& Fbo::operator=(Fbo&& rhs) {
	swap(*this, rhs);
	return *this;
}

void swap(Fbo& lhs, Fbo& rhs) {
	std::swap(lhs.id, rhs.id);
	std::swap(lhs.attachment_ids, rhs.attachment_ids);
}

void Fbo::bind()const {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	if (!attachment_ids.empty())
		glDrawBuffers((GLsizei)attachment_ids.size(), attachment_ids.data());
	else
		glDrawBuffer(GL_NONE);
}

void Fbo::addTextureAsColorbuffer(Tex const& img) {
	GLenum new_id = GL_COLOR_ATTACHMENT0 + GLint(attachment_ids.size());
	attachment_ids.emplace_back(new_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, new_id, GL_TEXTURE_2D, img.index(), 0);
}

void Fbo::addTextureAsDepthbuffer(Tex const& img) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, img.index(), 0);
}

void Fbo::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Fbo::check() const {
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {	
    // error
    std::string error = glbinding::Meta::getString(status);
    std::cerr <<  "OpenGL Error: glCheckFramebufferStatus(GL_FRAMEBUFFER)";
    std::cerr << " -> " << error << std::endl;
    // throw exception to allow for backtrace
    throw std::runtime_error("Execution of glCheckFramebufferStatus");
	}
}

// timer
Timer::Timer()
 :intervall{}
 ,startTime{}
{
	startTime = std::chrono::system_clock::now();
}

void Timer::update() {
	auto stoppedAt = std::chrono::system_clock::now();
	intervall = float(std::chrono::duration_cast<std::chrono::microseconds>(stoppedAt - startTime).count()) / (1000.0f * 1000.0f);
	startTime = stoppedAt;
}

/// Camera stuff
cameraSystem::cameraSystem(float delta, float mouseDelta, glm::vec3 pos)
 :position{pos}
 ,viewDir{glm::normalize(-position)}
 ,rightDir{glm::normalize(glm::cross(glm::vec3(viewDir), glm::vec3(0, 1, 0)))}
 ,upDir{0.0f, 1.0f, 0.0f}
 ,delta(delta)
 ,mouseDelta(mouseDelta)
 ,currentX{}
 ,currentY{}
{}

cameraSystem::cameraSystem(float delta, float mouseDelta, glm::vec3 pos, glm::vec3 dir, glm::vec3 up)
 :position{pos}
 ,viewDir{dir != glm::vec3(0,0,0) ? dir : -pos}
 ,rightDir{glm::cross(glm::vec3(viewDir), glm::vec3(up))}
 ,upDir{0.0f, 1.0f, 0.0f}
 ,delta(delta)
 ,mouseDelta(mouseDelta)
 ,currentX{}
 ,currentY{}
{}

void cameraSystem::moveForward(float delta) {
	position = position + (delta*viewDir);
}

void cameraSystem::moveBackward(float delta) {
	position = position - (delta*viewDir);
}

void cameraSystem::moveUp(float delta) {
	position = position + (delta*upDir);
}

void cameraSystem::moveDown(float delta) {
	position = position - (delta*upDir);
}

void cameraSystem::moveRight(float delta) {
	position = position + (delta*rightDir);
}

void cameraSystem::moveLeft(float delta) {
	position = position - (delta*rightDir);
}

void cameraSystem::yaw(float angle) {
	glm::mat3 R = glm::rotate(angle, glm::vec3(upDir)); 
	viewDir = R*viewDir;
	rightDir = R*rightDir;
}

void cameraSystem::pitch(float angle) {
	glm::mat3 R = glm::rotate(angle, glm::vec3(rightDir)); 
	viewDir = R*viewDir;
	// upDir = R*upDir;
}

void cameraSystem::roll(float angle) {
	glm::mat3 R = glm::rotate(angle, glm::vec3(viewDir)); 
	rightDir = R*rightDir;
	upDir = R*upDir;
}
