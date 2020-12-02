#include "triangle.h"

//constructor given  center, radius, and material
triangle::triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float tx0, float tx1, float tx2, float ty0, float ty1, float ty2, int m, scene* s) : rtObject(s)
{
	point0 = p0;
	point1 = p1;
	point2 = p2;

	texX0 = tx0;
	texX1 = tx1;
	texX2 = tx2;
	texY0 = ty0;
	texY1 = ty1;
	texY2 = ty2;
	matIndex = m;
	myScene = s;
}

//determinant of 3x3 matrix sending the entries where [a, b, c] is a column
float det3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
	return a * (e * i - f * h) - d * (b * i - c * h) + g * (b * f - c * e);
}

float triangle::testIntersection(glm::vec3 eye, glm::vec3 dir)
{
	//see the book/slides for a description of how to use Cramer's rule to solve
	//for the intersection(s) of a line and a plane, implement it here and
	//return the minimum distance (if barycentric coordinates indicate it hit
	//the triangle) otherwise 9999999

	//triangle points
	glm::vec3 A = point0, B = point1, C = point2;
	
	//matrix stuff
	float a, b, c, d, e, f, g, h, i, j, k, l, beta, gamma, t, M;
	
	//kramers rule stuff to reduce number of computations
	float eiMinusHf, gfMinusDi, dhMinusEg, akMinusJb, jcMinusAl, blMinusKc;
	
	//setting the dummy variables for the matrix to use Cramer's rule
	a = A.x - B.x;
	b = A.y - B.y;
	c = A.z - B.z;

	d = A.x - C.x;
	e = A.y - C.y;
	f = A.z - C.z;

	g = dir.x;
	h = dir.y;
	i = dir.z;

	j = A.x - eye.x;
	k = A.y - eye.y;
	l = A.z - eye.z;
	
	eiMinusHf = e * i - h * f;
	gfMinusDi = g * f - d * i;
	dhMinusEg = d * h - e * g;
	akMinusJb = a * k - j * b;
	jcMinusAl = j * c - a * l;
	blMinusKc = b * l - k * c;
	
	M = a * eiMinusHf + b * gfMinusDi + c * dhMinusEg;

	//book has interval for [t_0, t_1] for early termination but unsure what would be a good interval (I believe this is a clip plane in first submission)
	beta	=  (j * eiMinusHf + k * gfMinusDi + l * dhMinusEg) / M;
	gamma	=  (i * akMinusJb + h * jcMinusAl + g * blMinusKc) / M;
	t		= -(f * akMinusJb + e * jcMinusAl + d * blMinusKc) / M;

	// add other early termination conditions


	//solution on the plane inside the triangle
	if (beta > 0.0f && gamma > 0.0f && beta + gamma < 1.0f && t >= 0.0f)
		return t;

	//triangle is degenerate, ray intersects the plane outside of the trianlge, or the ray is parallel to the plane
	return 9999999;
}

glm::vec3 triangle::getNormal(glm::vec3 eye, glm::vec3 dir, float t)
{
	//construct the barycentric coordinates for the plane
	glm::vec3 bary1 = point1 - point0;
	glm::vec3 bary2 = point2 - point0;

	//cross them to get the normal to the plane
	//note that the normal points in the direction given by right-hand rule
	//(this can be important for refraction to know whether you are entering or leaving a material)
	glm::vec3 normal = glm::normalize(glm::cross(bary1, bary2));
	return normal;
}

glm::vec2 triangle::getTextureCoords(glm::vec3 eye, glm::vec3 dir)
{
	//find alpha and beta (parametric distance along barycentric coordinates)
	//use these in combination with the known texture surface location of the vertices
	//to find the texture surface location of the point you are seeing

	//triangle points - just using ABC names for ease of reading
	glm::vec3 A = point0, B = point1, C = point2;

	//matrix stuff
	float a, b, c, d, e, f, g, h, i, j, k, l, alpha, beta, gamma, t, M;

	//kramers rule stuff to reduce number of computations
	float eiMinusHf, gfMinusDi, dhMinusEg, akMinusJb, jcMinusAl, blMinusKc;

	//setting the dummy variables for the matrix to use Cramer's rule
	a = A.x - B.x;
	b = A.y - B.y;
	c = A.z - B.z;

	d = A.x - C.x;
	e = A.y - C.y;
	f = A.z - C.z;

	g = dir.x;
	h = dir.y;
	i = dir.z;

	j = A.x - eye.x;
	k = A.y - eye.y;
	l = A.z - eye.z;

	eiMinusHf = e * i - h * f;
	gfMinusDi = g * f - d * i;
	dhMinusEg = d * h - e * g;
	akMinusJb = a * k - j * b;
	jcMinusAl = j * c - a * l;
	blMinusKc = b * l - k * c;

	M = a * eiMinusHf + b * gfMinusDi + c * dhMinusEg;

	beta	= (j * eiMinusHf + k * gfMinusDi + l * dhMinusEg) / M;
	gamma	= (i * akMinusJb + h * jcMinusAl + g * blMinusKc) / M;
	alpha	= 1.0f - beta - gamma;

	glm::vec2 texAlpha, texBeta, texGamma;
	texAlpha = glm::vec2(texX0, texY0);
	texBeta  = glm::vec2(texX1, texY1);
	texGamma = glm::vec2(texX2, texY2);

	//I feel like we could get some speed up with optional alpha, beta, and gamma params since we
	//already found them in the testIntersection() function, so I think I'll add that
	return alpha * texAlpha + beta * texBeta + gamma * texGamma;
}