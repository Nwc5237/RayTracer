#pragma once
#include "rtObject.h"
#include "scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class triangle : public rtObject
{
public:
	//constructor given points in 3d space and corresponding
	//locations on the texture surface
	triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float tx0, float tx1, float tx2, float ty0, float ty1, float ty2, int m, scene* s);

	//see descriptions in rtObject
	float testIntersection(glm::vec3 eye, glm::vec3 dir);
	glm::vec3 getNormal(glm::vec3 eye, glm::vec3 dir, float t = -1.0f);
	glm::vec2 getTextureCoords(glm::vec3 eye, glm::vec3 dir);

private:
	//the points in 3d space that define the triangle
	glm::vec3 point0, point1, point2;
	//the x-y locations on the texture image corresponding to each vertex
	float texX0, texX1, texX2, texY0, texY1, texY2;
};
