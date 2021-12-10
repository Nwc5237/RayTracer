#include "scene.h"

scene::scene(const char* filename)
{
	parse(filename);
	std::cout << std::endl << "background: " << glm::to_string(bgColor);
	std::cout << std::endl << "ambient: " << glm::to_string(ambLight);
	std::cout << std::endl << "eye: " << glm::to_string(eye);
	std::cout << std::endl << "lookAt: " << glm::to_string(lookAt);
	std::cout << std::endl << "up: " << glm::to_string(up);
	std::cout << std::endl << "fovy: " << fovy;
	std::cout << std::endl;
}

float clamp(float num, float min, float max)
{
	if (num < min)
		return min;
	if (num > max)
		return max;

	return num;
}

//takes in normal, halway vector, and roughness alpha value
float DistributionGGX(glm::vec3 N, glm::vec3 H, float a)
{
	a = a * a; //adding because apparently roughness should be squared according to disney and epic
	float a2, NdotH, NdotH2, num, denom;
	a2 = a * a;
	NdotH = glm::dot(N, H);
	NdotH2 = NdotH * NdotH;

	num = a2;
	denom = 3.14159 * glm::pow((NdotH2 * (a2 - 1) + 1), 2);
	return num / denom;
}

//normal, view, and roughness alpha
float GeometrySchlickGGX(glm::vec3 N, glm::vec3 v, float a)
{
	a = a * a; //adding because apparently roughness should be squared according to disney and epic
	float k, NdotV;
	k = glm::pow(a + 1, 2) / 8.0f; //remaps differently if we do IBL --- becomes (a^2)/2
	NdotV = glm::max(glm::dot(N, v), 0.0f);

	return NdotV / (NdotV * (1 - k) + k);
}

//normal, view, light (to/from surface) and alpha
float GeometrySmith(glm::vec3 N, glm::vec3 v, glm::vec3 l, float a)
{
	a = a * a; //adding because apparently roughness should be squared according to disney and epic
	return GeometrySchlickGGX(N, v, a) * GeometrySchlickGGX(N, l, a);
}

//incidence angle (from dot product of normal and halfway vector, index of refraction
float FresnelSchlick(float VdotH, float ior)
{
	//float F0 = glm::pow((ior - 1) / (ior + 1), 2.0f);
	float F0 = .04; //------------------------------------------------------------------------------------------------------------------try using this
	return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotH, 0.0f, 1.0f), 5.0);
}

float Attenuate(glm::vec3 lightPos, glm::vec3 surface)
{
	return 1.0f / glm::pow(glm::length(lightPos-surface), 2.0f);
}

float Radiance(glm::vec3 surface, glm::vec3 lightPos, glm::vec3 N)
{
	glm::vec3 wi = glm::normalize(lightPos - surface);
	float cosTheta = glm::max(glm::dot(N, wi), 0.0f);
	float attenuation = Attenuate(lightPos, surface);
	return attenuation * cosTheta;
}

glm::vec3 pbrLighting()
{
	glm::vec3 result;
	float kS, kD; //calculate specular and diffuse components
	//kS = getSpecularPBR();
	//kD = 1.0f - kS;

	return result;
}

glm::vec3 scene::rayTrace(glm::vec3 eye, glm::vec3 dir, int recurseDepth)
{
	//start with black, add color as we go
	glm::vec3 answer(0.0f);

	//test for intersection against all our objects
	float dist = myObjGroup->testIntersections(eye, dir);
	//if we saw nothing, return the background color of our scene
	if (dist == 9999999)
		return bgColor;

	//get the material index and normal vector(at the point we saw) of the object we saw
	int matIndex = myObjGroup->getClosest()->getMatIndex();
	material* texture = &myMaterials.at(matIndex);
	glm::vec3 normal = myObjGroup->getClosest()->getNormal(eye, dir);

	//determine texture color
	glm::vec3 textureColor;

	if (!texture->image)
		//this is multiplicative, rather than additive
		//so if there is no texture, just use ones
		textureColor = glm::vec3(1.0f);
	else
	{
		//if there is a texture image, ask the object for the image coordinates (between 0 and 1)
		glm::vec2 coords = myObjGroup->getClosest()->getTextureCoords(eye, dir);

		//get the color from that image location

		int x = (int)(texture->width * coords.x);
		int y = (int)(texture->height * coords.y);
		textureColor = texture->data[x + y * texture->width];
	}

	//add diffuse color times ambient light to our answer
	//if (dist < 9999999)
		answer += texture->diffuseCol * ambLight;

		bool doPBR = true;
		answer = glm::vec3(0);
	//iterate through all lights
	for (int i = 0; i < myLights.size(); i++)
	{
		glm::vec3 k_s, k_d, I, n, l, h, surface;
		k_d = texture->diffuseCol;
		k_s = texture->specularCol;
		I = myLights[i].color;
		n = normal;
		surface = eye + dist * dir;
		l = glm::normalize(myLights[i].position - surface);
		h = glm::normalize(l - dir);

		if (!doPBR)
		{
			//if the light can see the surface point,
			//add its diffuse color to a total diffuse for the point (using our illumination model)
			//use testIntersection to help decide this
			float p = texture->shininess;

			float t_shadow = myObjGroup->testIntersections(surface + l * .001f, l);
			if (t_shadow == 9999999)
				//answer += k_d * I * glm::max(0.0f, glm::dot(n, l)) + k_s * I * glm::pow(glm::max(0.0f, glm::dot(n, h)), p);
				answer += k_d * I + k_s * I * glm::pow(glm::max(0.0f, glm::dot(n, h)), p);

		}
		else
		{
			float fader = 2000.0f;
			glm::vec3 v = -dir;
			float pi = 3.14159;
			bool useTex = false;
			glm::vec3 num;
			float myAlph, alpha, D, G, f, denom;
			myAlph = 0.5f;
			alpha = useTex ? glm::length(texture->reflectiveCol) : myAlph;
			
			D = DistributionGGX(normal, h, alpha); //texture->reflectiveCol[0]
			G = GeometrySmith(normal, v, l, alpha);
			f = FresnelSchlick(glm::dot(v, n), texture->refractionIndex); //dir and normal from textbook, learnopengl also uses halfway in place of dir and I think that's wrong

			glm::vec3 F(f);
			num = D * G * F;
			denom = 4.0f * glm::max(glm::dot(normal, v), 0.0f) * glm::max(glm::dot(normal, l), 0.0f) + .00001f; //adding teeny term at the end so we don't divide by zero
			glm::vec3 spec = num / denom;

			glm::vec3 kS, kD;
			kS = F;
			kD = glm::vec3(1.0f) - kS;

			glm::vec3 radiance = fader * myLights[i].color * Attenuate(myLights[i].position, surface);
			float NdotL = glm::max(glm::dot(normal, l), 0.0f);

			glm::vec3 Lo = (kD * texture->diffuseCol / pi + spec) * radiance * NdotL;

			answer = Lo;
			//answer = kD * diff + spec;
			//answer = D * G * texture->diffuseCol + F*texture->specularCol;
			//answer = F * texture->diffuseCol + D * texture->diffuseCol + G * texture->diffuseCol;

		}
		//add the diffuse light times the accumulated diffuse light to our answer
	}
	//put a limit on the depth of recursion
	if (recurseDepth<3)
	{
	//reflect our view across the normal
	//recusively raytrace from the surface point along the reflected view
	//add the color seen times the reflective color
		glm::vec3 surface = eye + dist * dir;
		glm::vec3 r = glm::normalize(dir - 2.0f * glm::dot(dir, normal) * normal);
		answer += texture->reflectiveCol * rayTrace(surface + r * .001f, r, recurseDepth+1);


	//if going into material (dot prod of dir and normal is negative), bend toward normal
	//find entry angle using inverse cos of dot product of dir and -normal
	//multiply entry angle by index of refraction to get exit angle
	//else, bend away
	//find entry angle using inverse cos of dot product of dir and normal
	//divide entry angle by index of refraction to get exit angle
	//recursively raytrace from the other side of the object along the new direction
	//add the color seen times the transparent color
		
		float theta, phi, n, n_t;
		glm::vec3 t;

		if (glm::dot(dir, normal) < 0)
		{
			theta = acos(glm::dot(dir, -normal));
			phi = theta * texture->refractionIndex;
			n = 1.0f;
			n_t = texture->refractionIndex;

			t = n * (dir + normal * cos(theta)) / n_t - normal * cos(phi);
		}
		else
		{
			theta = acos(glm::dot(dir, normal));
			phi = theta / texture->refractionIndex;
			n = texture->refractionIndex;
			n_t = 1.0f;

			t = n * (dir + normal * cos(theta)) / n_t - normal * cos(phi);
		}

		t = glm::normalize(t);
		answer += rayTrace(surface + .001f * t, t, recurseDepth + 1) * texture->transparentCol;
	}

	//multiply whatever color we have found by the texture color
	answer *= textureColor;
	return answer;
}

glm::vec3 scene::getEye()
{
	return eye;
}

glm::vec3 scene::getLookAt()
{
	return lookAt;
}

glm::vec3 scene::getUp()
{
	return up;
}

float scene::getFovy()
{
	return fovy;
}

void scene::parse(const char *filename)
{
	//some default values in case parsing fails
	myObjGroup = NULL;
	bgColor = glm::vec3(0.5, 0.5, 0.5);
	ambLight = glm::vec3(0.5, 0.5, 0.5);
	eye = glm::vec3(0, 0, 0);
	lookAt = glm::vec3(0, 0, -1);
	up = glm::vec3(0, 1, 0);
	fovy = 45;
	file = NULL;
	curline = 1;

	//the file-extension needs to be "ray"
	assert(filename != NULL);
	const char *ext = &filename[strlen(filename) - 4];
	// if no ray extention
	if (strcmp(ext, ".ray"))
	{
		printf("ERROR::SCENE::FILE_NOT_SUCCESFULLY_READ::\"%s\"\n", filename);
		assert(!strcmp(ext, ".ray"));
	}
	file = fopen(filename, "r");
	// Fails if no file exists
	if (file == NULL)
	{
		printf("ERROR::SCENE::FILE_NOT_SUCCESFULLY_READ::\"%s\"\n", filename);
		//assert(file != NULL);
	}
	char token[MAX_PARSER_TOKEN_LENGTH];

	//prime the parser pipe
	parse_char = fgetc(file);

	while (getToken(token))
	{
		if (!strcmp(token, "Background"))
			parseBackground();
		else if (!strcmp(token, "Camera"))
			parseCamera();
		else if (!strcmp(token, "Materials"))
			parseMaterials();
		else if (!strcmp(token, "Group"))
			myObjGroup = parseGroup();
		else if (!strcmp(token, "Lights"))
			parseLights();
		else
		{
			std::cout << "Unknown token in parseFile: '" << token <<
				"' at input line " << curline << "\n";
			exit(-1);
		}
	}
}

/* Parse the "Camera" token */
void scene::parseCamera()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert(!strcmp(token, "{"));

	//get the eye, center, and up vectors (similar to gluLookAt)
	getToken(token); assert(!strcmp(token, "eye"));
	eye = readVec3f();

	getToken(token); assert(!strcmp(token, "lookAt"));
	lookAt = readVec3f();

	getToken(token); assert(!strcmp(token, "up"));
	up = readVec3f();


	getToken(token); assert(!strcmp(token, "fovy"));
	fovy = readFloat();

	getToken(token); assert(!strcmp(token, "}"));
}

/* Parses the "Background" token */
void scene::parseBackground()
{
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token); assert(!strcmp(token, "{"));
	while (1)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			break;
		}
		else if (!strcmp(token, "color"))
		{
			bgColor = readVec3f();
		}
		else if (!strcmp(token, "ambientLight"))
		{
			ambLight = readVec3f();
		}
		else
		{
			std::cout << "Unknown token in parseBackground: " << token << "\n";
			assert(0);
		}
	}
}

/* Parses the "Group" token */
rtObjGroup* scene::parseGroup()
{
	char token[MAX_PARSER_TOKEN_LENGTH];


	getToken(token);
	assert(!strcmp(token, "{"));

	/**********************************************/
	/* Instantiate the group object               */
	/**********************************************/
	rtObjGroup *answer = new rtObjGroup();

	bool working = true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			working = false;
		}
		else
		{
			if (!strcmp(token, "Sphere"))
			{
				sphere *sceneElem = parseSphere();
				assert(sceneElem != NULL);
				answer->addObj(sceneElem);
			}
			else if (!strcmp(token, "Triangle"))
			{
				triangle *sceneElem = parseTriangle();
				assert(sceneElem != NULL);
				answer->addObj(sceneElem);
			}
			else
			{
				std::cout << "Unknown token in Group: '" << token << "' at line "
					<< curline << "\n";
				exit(0);
			}
		}
	}

	/* Return the group */
	return answer;
}

/* Parse the "Sphere" token */
sphere* scene::parseSphere()
{
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token); assert(!strcmp(token, "{"));
	getToken(token); assert(!strcmp(token, "materialIndex"));
	int sphere_material_index = readInt();

	getToken(token); assert(!strcmp(token, "center"));
	glm::vec3 center = readVec3f();
	getToken(token); assert(!strcmp(token, "radius"));
	float radius = readFloat();
	getToken(token); assert(!strcmp(token, "}"));

	std::printf("Sphere:\n\tCenter: %s\n", glm::to_string(center).c_str());
	std::printf("\tRadius: %f\n", radius);
	std::printf("\tSphere Material Index: %d\n\n", sphere_material_index);

	/**********************************************/
	/* The call to your own constructor goes here */
	/**********************************************/
	return new sphere(center, radius, sphere_material_index, this);
}

/* Parse out the "Triangle" token */
triangle* scene::parseTriangle()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert(!strcmp(token, "{"));

	/* Parse out vertex information */
	getToken(token); assert(!strcmp(token, "vertex0"));
	glm::vec3 v0 = readVec3f();
	getToken(token); assert(!strcmp(token, "vertex1"));
	glm::vec3 v1 = readVec3f();
	getToken(token); assert(!strcmp(token, "vertex2"));
	glm::vec3 v2 = readVec3f();
	getToken(token); assert(!strcmp(token, "tex_xy_0"));
	float x0 = 0;
	float y0 = 0;
	x0 = readFloat();
	y0 = readFloat();
	getToken(token); assert(!strcmp(token, "tex_xy_1"));
	float x1 = 0;
	float y1 = 0;
	x1 = readFloat();
	y1 = readFloat();
	getToken(token); assert(!strcmp(token, "tex_xy_2"));
	float x2 = 0;
	float y2 = 0;
	x2 = readFloat();
	y2 = readFloat();
	getToken(token); assert(!strcmp(token, "materialIndex"));
	int mat = readInt();

	getToken(token); assert(!strcmp(token, "}"));


	std::printf("Triangle:\n");
	std::printf("\tVertex0: %s tex xy 0 %s\n", glm::to_string(v0).c_str(), glm::to_string(glm::vec2(x0, y0)).c_str());
	std::printf("\tVertex1: %s tex xy 1 %s\n", glm::to_string(v1).c_str(), glm::to_string(glm::vec2(x1, y1)).c_str());
	std::printf("\tVertex1: %s tex xy 1 %s\n", glm::to_string(v2).c_str(), glm::to_string(glm::vec2(x2, y2)).c_str());
	std::printf("\tTriangle Material Index: %d\n\n", mat);

	/**********************************************/
	/* The call to your own constructor goes here */
	/**********************************************/
	return new triangle(v0, v1, v2, x0, x1, x2, y0, y1, y2, mat, this);
}

/* Parse the "Materials" token */
void scene::parseMaterials()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	char texname[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert(!strcmp(token, "{"));

	/* Loop over each Material */
	bool working = true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			working = false;
		}
		else if (!strcmp(token, "Material"))
		{
			getToken(token); assert(!strcmp(token, "{"));
			texname[0] = '\0';
			glm::vec3 diffuseColor(1, 1, 1);
			glm::vec3 specularColor(0, 0, 0);
			float shininess = 1;
			glm::vec3 transparentColor(0, 0, 0);
			glm::vec3 reflectiveColor(0, 0, 0);
			float indexOfRefraction = 1;

			while (1)
			{
				getToken(token);
				if (!strcmp(token, "textureFilename"))
				{
					getToken(token);
					strcpy(texname, token);
				}
				else if (!strcmp(token, "diffuseColor"))
					diffuseColor = readVec3f();
				else if (!strcmp(token, "specularColor"))
					specularColor = readVec3f();
				else if (!strcmp(token, "shininess"))
					shininess = readFloat();
				else if (!strcmp(token, "transparentColor"))
					transparentColor = readVec3f();
				else if (!strcmp(token, "reflectiveColor"))
					reflectiveColor = readVec3f();
				else if (!strcmp(token, "indexOfRefraction"))
					indexOfRefraction = readFloat();
				else
				{
					assert(!strcmp(token, "}"));
					break;
				}
			}

			material temp;

			std::printf("Material:\n", texname);
			temp.diffuseCol = diffuseColor;
			std::printf("\tDiffuse Color: %s\n", glm::to_string(diffuseColor).c_str());
			temp.specularCol = specularColor;
			std::printf("\tSpecular Color: %s\n", glm::to_string(specularColor).c_str());
			temp.shininess = shininess;
			std::printf("\tShininess: %f\n", shininess);
			temp.transparentCol = transparentColor;
			std::printf("\tTransparent Color: %s\n", glm::to_string(transparentColor).c_str());
			temp.reflectiveCol = reflectiveColor;
			std::printf("\tReflective Color: %s\n", glm::to_string(reflectiveColor).c_str());
			temp.refractionIndex = indexOfRefraction;
			std::printf("\tIndex Of Refraction: %f\n", indexOfRefraction);
			std::printf("\tFileName: %s\n\n", texname);
			if (strcmp(texname, "NULL"))
			{
				temp.image = true;
				unsigned char *raw_data = stbi_load(texname, &temp.width, &temp.height, &temp.nrComponents, 0);
				if (raw_data)
				{
					// Make vector for each image
					for (int y = 0; y < temp.height; y++)
						for (int x = 0; x < temp.width; x++)
							if (temp.nrComponents == 1)
								temp.data.push_back(glm::vec3((float)raw_data[x + y*temp.height]));
							else
								temp.data.push_back(glm::vec3((float)raw_data[temp.nrComponents * (x + y*temp.width)],
									(float)raw_data[temp.nrComponents * (x + y*temp.width) + 1],
									(float)raw_data[temp.nrComponents * (x + y*temp.width) + 2]) / 255.0f);
					stbi_image_free(raw_data);
				}
				else
				{
					std::cout << "Texture failed to load at path: " << texname << std::endl;
					stbi_image_free(raw_data);
				}
			}
			else
				temp.image = false;

			myMaterials.push_back(temp);

		}
	}
}

void scene::parseLights()
{
	char token[MAX_PARSER_TOKEN_LENGTH];
	char texname[MAX_PARSER_TOKEN_LENGTH];
	getToken(token); assert(!strcmp(token, "{"));

	/* Loop over each Material */
	bool working = true;
	while (working)
	{
		getToken(token);
		if (!strcmp(token, "}"))
		{
			working = false;
		}
		else if (!strcmp(token, "Light"))
		{
			getToken(token); assert(!strcmp(token, "{"));
			texname[0] = '\0';
			glm::vec3 position(0, 0, 0);
			glm::vec3 color(1, 1, 1);

			while (1)
			{
				getToken(token);
				if (!strcmp(token, "position"))
					position = readVec3f();
				else if (!strcmp(token, "color"))
					color = readVec3f();
				else
				{
					assert(!strcmp(token, "}"));
					break;
				}
			}

			/**********************************************/
			/* The call to your own constructor goes here */
			/**********************************************/
			light temp;
			temp.position = position;
			std::printf("Light:\n\tPostion: %s\n", glm::to_string(position).c_str());
			temp.color = color;
			std::printf("\Color: %s\n\n", glm::to_string(color).c_str());
			myLights.push_back(temp);

		}
	}
}

/* consume whitespace */
void scene::eatWhitespace(void)
{
	bool working = true;

	do {
		while (isspace(parse_char))
		{
			if (parse_char == '\n')
			{
				curline++;
			}
			parse_char = fgetc(file);
		}

		if ('#' == parse_char)
		{
			/* this is a comment... eat until end of line */
			while (parse_char != '\n')
			{
				parse_char = fgetc(file);
			}

			curline++;
		}
		else
		{
			working = false;
		}

	} while (working);
}

/* Parse out a single token */
int scene::getToken(char token[MAX_PARSER_TOKEN_LENGTH])
{
	int idx = 0;

	assert(file != NULL);
	eatWhitespace();

	if (parse_char == EOF)
	{
		token[0] = '\0';
		return 0;
	}
	while ((!(isspace(parse_char))) && (parse_char != EOF))
	{
		token[idx] = parse_char;
		idx++;
		parse_char = fgetc(file);
	}

	token[idx] = '\0';
	return 1;
}

/* Reads in a 3-vector */
glm::vec3 scene::readVec3f()
{	
	float a = readFloat();
	float b = readFloat();
	float c = readFloat();
	return glm::vec3(a, b, c);
}

/* Reads in a single float */
float scene::readFloat()
{
	float answer;
	char buf[MAX_PARSER_TOKEN_LENGTH];

	if (!getToken(buf))
	{
		std::cout << "Error trying to read 1 float (EOF?)\n";
		assert(0);
	}

	int count = sscanf(buf, "%f", &answer);
	if (count != 1)
	{
		std::cout << "Error trying to read 1 float\n";
		assert(0);

	}
	return answer;
}

/* Reads in a single int */
int scene::readInt()
{
	int answer;
	char buf[MAX_PARSER_TOKEN_LENGTH];

	if (!getToken(buf))
	{
		std::cout << "Error trying to read 1 int (EOF?)\n";
		assert(0);
	}

	int count = sscanf(buf, "%d", &answer);
	if (count != 1)
	{
		std::cout << "Error trying to read 1 int\n";
		assert(0);

	}
	return answer;
}
