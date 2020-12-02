#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct material
{
	//the image loaded using st
	bool image;
	std::vector<glm::vec3> data;
	int width, height, nrComponents;
	//four RGB colors for different material properties
	glm::vec3 diffuseCol, specularCol, transparentCol, reflectiveCol;

	//the shininess (see illumination model)
	//and refraction index (look up refraction of light through materials)
	float shininess, refractionIndex;
};