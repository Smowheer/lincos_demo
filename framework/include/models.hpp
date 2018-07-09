#pragma once

#include <vector>
#include <string>

#include <glm/gtc/type_precision.hpp>

// Screen Space Quad
class simpleQuad
{
public:
	simpleQuad();
	~simpleQuad();
	void upload();
	void draw() const;
protected:
	std::vector<uint32_t> indices;
	std::vector<glm::vec3> vertices;
	uint32_t vbo[2];
};

class simplePoint
{
public:
	simplePoint();
	~simplePoint();
	void upload();
	void draw() const;
protected:
	glm::vec3 vertex;
	uint32_t vbo;
};


// very simple geometry
class simpleModel
{
public:
	simpleModel(std::string const& fileName);
	~simpleModel();
	void draw() const;
protected:
	simpleModel();
	void upload();
	std::vector<uint32_t> indices;
public:
	std::vector<glm::vec3> vertices;
protected:
	std::vector<glm::vec3> normals;
	uint32_t vbo[3];
};

class groundPlane : public simpleModel
{
public:
	groundPlane(const float height, const float width);
};

class shurikenModel : public simpleModel
{
public:
  shurikenModel();
};

class solidTorus : public simpleModel 
{
public:
	solidTorus(const float r, const float R, const float sides, const float rings);
};

class solidSphere
{
public:
	solidSphere(const float radius, const int slices, const int stacks);
	~solidSphere();
	void draw() const;
protected:
	void upload();
	std::vector<uint32_t> indices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> vertices;
	uint32_t vbo[3];
};
