#include "sphere.h"

//epsilon value to determine equality
float epsilon = .00001f;

//constructor given  center, radius, and material
sphere::sphere(glm::vec3 c, float r, int m, scene* s) : rtObject(s)
{
	center = c;
	radius = r;
	matIndex = m;
	myScene = s;
}

//a little function to check 'equality' within an epsilon band
bool equals(float a, float b)
{
	return glm::abs(a - b) < epsilon;
}

float sphere::testIntersection(glm::vec3 eye, glm::vec3 dir)
{
	//see the book for a description of how to use the quadratic rule to solve
	//for the intersection(s) of a line and a sphere, implement it here and
	//return the minimum positive distance or 9999999 if none

	//it might be nice to save even more values, like eye - center and glm::sqrt(discriminant)
	float A, B, C, discriminant, t1, t2, sqrtOfDiscriminant;
	glm::vec3 eyeMinusCenter;
	dir = glm::normalize(dir);
	eyeMinusCenter = eye - center;
	
	A = glm::dot(dir, dir);
	B = glm::dot(dir, eyeMinusCenter);
	C = glm::dot(eyeMinusCenter, eyeMinusCenter) - radius * radius;
	discriminant = B * B - A * C;
	sqrtOfDiscriminant = glm::sqrt(discriminant);
	if (equals(discriminant, 0))
	{
		//hit tangentially (one intersection), no light gets reflected
		return 9999999;
	}
	else if (discriminant >= epsilon)
	{
		//general case of two intersections
		t1 = (-B + sqrtOfDiscriminant) / A;
		t2 = (-B - sqrtOfDiscriminant) / A;

		
		//if one is negative and the other is positive, we are inside the sphere, and so the negative one is behind the eye. return the positive t value
		if (t1 < 0.0f && t2 < 0.0f)
			return 9999999;

		if (t1 < t2)
		{
			if (t1 < 0.0f)
				return t2;
			return t1;
		}
		if (t2 < 0.0f)
			return t1;
		return t2;
	}
	else
	{
		//no solutions so no hits
		return 9999999;
	}
}


/*
 * Before calling this testIntersection() should've been called, so send the
 * t value so that we don't compute intersections any more than is necessaey,
 * and make sure not to go through with the call when the value is 9999999
 */
glm::vec3 sphere::getNormal(glm::vec3 eye, glm::vec3 dir, float t)
{
	//once you can test for intersection,
	//simply add (distance * view direction) to the eye location to get surface location,
	//then subtract the center location of the sphere to get the normal out from the sphere
	glm::vec3 normal;
	if(t == -1.0f)
		t = testIntersection(eye, dir); //optional param to not recompute intersections when already computed
	
	//normalize the vector from the center to the point on the surface and you've got yourself a normal vector bucko
	normal = glm::normalize((eye + t * dir) - center);
	
	return normal;
}

float map(float x, float min1, float max1, float min2, float max2)
{
	return min2 + (max2 - min2) * ((x - min1) / (max1 - min1));
}

glm::vec2 sphere::getTextureCoords(glm::vec3 eye, glm::vec3 dir)
{
	//find the normal as in getNormal

	//use these to find spherical coordinates
	glm::vec3 x(1, 0, 0);
	glm::vec3 z(0, 0, 1);

	//phi is the angle down from z
	//theta is the angle from x curving toward y
	//hint: angle between two vectors is the acos() of the dot product
	float phi, theta, PI = 3.1415926535897;
	glm::vec3 projection, normal;

	//find phi using normal and z
	normal = getNormal(eye, dir);
	phi = glm::acos(glm::dot(normal, z));

	//find the x-y projection of the normal
	projection = glm::vec3(normal.x, normal.y, 0);

	//find theta using the x-y projection and x
	theta = glm::acos(glm::dot(projection, x));

	if (projection.y < 0)
		theta = 2 * PI - theta;

	//if x-y projection is in quadrant 3 or 4, then theta=2*PI-theta

	//return coordinates scaled to be between 0 and 1 --- How so? what range fo we scale from?
	return glm::vec2(theta/(2*PI), phi / PI);
	//return glm::vec2(map(theta, 0.0f, 2 * PI, 0.0f, 1.0f), map(phi, 0.0f, PI, 0.0f, 1.0f));//glm::vec2(phi, theta);
}