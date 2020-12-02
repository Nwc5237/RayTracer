#pragma once
#include "rtObject.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class rtObject;

//object is a virtual class: it is never used, except as a subclass
class rtObjGroup
{
public:
	//default constructor
	rtObjGroup();

	//add an object to the vector
	void addObj(rtObject* o);

	//returns the distance from the eye to the point of
	//intersection with the closest object in the group
	//or 9999999 if no intersection exists
	float testIntersections(glm::vec3 eye, glm::vec3 dir);

	//accessor for objects in the vector
	rtObject* getObj(int index);

	//accessor for the object that was closest
	//during the last raytrace call
	rtObject* getClosest();

private:
	//holds all the objects that are part of this group
	std::vector<rtObject*> myObjects;

	int indexOfClosest;
};
