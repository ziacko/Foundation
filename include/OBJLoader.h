#pragma once
#include "VertexAttribute.h"

/*
enum VB_TYPES {
	INDEX_BUFFER,
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	NUM_VBs
};*/

struct OBJMesh
{
	std::string							name;

	std::vector<vertexAttribute_t>		vertices;
	std::vector<glm::vec3>				tempVertices;
	std::vector<unsigned int>			originalIndices;
	std::vector<unsigned int>			indices;
	std::vector<texture>				textures;


	unsigned int						vertexBufferHandle;
	unsigned int						indexBufferHandle;

	unsigned int						vertexOffset;
	unsigned int						indexOffset;

	unsigned int						numVertices;
	unsigned int						numIndices;
	unsigned int						vertexArrayHandle;

	OBJMesh()
	{
		textures = std::vector<texture>();

		vertexOffset = 0;
		indexOffset = 0;
	}

	OBJMesh(std::vector<vertexAttribute_t> inVertices, std::vector<unsigned int> inIndices, std::vector<texture> inTextures) :
		textures(inTextures)
	{
		vertexOffset = 0;
		indexOffset = 0;
	}
};

class OBJModel
{
public:
	
	std::string					resourcePath;
	std::string					directory;

	glm::vec3					position;
	glm::vec3					scale;
	glm::vec3					rotation;

	std::vector<glm::vec3>		positions;
	std::vector<glm::vec3>		normals;
	std::vector<glm::vec2>		texCoords;
	std::vector<int>			positionIndices;
	std::vector<int>			normalIndices;
	std::vector<int>			uvIndices;
	unsigned int				vertexArrayHandle;

	GLuint bufferTypes[NUM_VBs];
	std::vector<OBJMesh>			meshes;

	OBJModel(std::string resourcePath = "../../resources/models/House.obj")
	{
		this->resourcePath =	resourcePath;

		position = glm::vec3(0);
		scale	= glm::vec3(1.0f);
		rotation = glm::vec3(0.0f);
	}

	void Load()
	{
		directory = resourcePath.substr(0, resourcePath.find_last_of('/'));



		LoadFromFile();
		//LoadIntoGL();
		DefineBuffers();

		glBindVertexArray(0);
		glBindBuffer(gl_array_buffer, 0);
		glBindBuffer(gl_element_array_buffer, 0);
	}

	void LoadFromFile()
	{

		char wholeLine[256];
		FILE* file = fopen(resourcePath.c_str(), "r");

		if (file == nullptr || file == NULL)
		{
			printf("unable to open file %s", resourcePath.c_str());
		}

		int maxVertexIndex = 0;
		int maxUVIndex = 0;
		int maxNormalIndex = 0;
		bool negative = false;
		bool nextMesh = false;
		int test = 0;
		//check the first few characters per line
		for (size_t iter = 0; iter != EOF;)
		{
			vertexAttribute_t vertex;
			static unsigned int polygon[3] = {0};
			static unsigned char polygonIter = 0;
			bool genIndices = false;
			
			if(!nextMesh)
			{
				iter = fscanf(file, "%s", wholeLine);
			}
			
			//if line starts with V, read in the position data
			if (!strcmp(wholeLine, "v"))
			{
				fscanf(file, "%f %f %f\n", &vertex.position.x, &vertex.position.y, &vertex.position.z);
				positions.push_back(vertex.position);

			}

			//if the line starts with VT, read in the UV data
			if (!strcmp(wholeLine, "vt"))
			{
				fscanf(file, "%f %f\n", &vertex.uv.x, &vertex.uv.y);
				texCoords.push_back(vertex.uv);
			}

			//if the line starts with vn, read in the vertex normal data
			if (!strcmp(wholeLine, "vn"))
			{
				fscanf(file, "%f %f %f\n", &vertex.normal.x, &vertex.normal.y, &vertex.normal.z);
				normals.push_back(vertex.normal);
			}

			//maybe treat these as seperate meshes?
			if (!strcmp(wholeLine, "g"))
			{
				OBJMesh newMesh;
				nextMesh = false;
				
				unsigned int maxVertexRelativeIndex = 0;

				//get name of this mesh
				fscanf(file, "%s", newMesh.name.c_str());
				for (size_t indexIter = 0; indexIter != EOF; indexIter = fscanf(file, "%s", wholeLine))
				{
					if (!strcmp(wholeLine, "f"))
					{
						int vertexIndex[4], normalIndex[4], uvIndex[4] = { 0 };

						vertexAttribute_t attributes[4] = {};

						int matches = fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i\n",
							&vertexIndex[0], &uvIndex[0], &normalIndex[0],
							&vertexIndex[1], &uvIndex[1], &normalIndex[1],
							&vertexIndex[2], &uvIndex[2], &normalIndex[2],
							&vertexIndex[3], &uvIndex[3], &normalIndex[3]);

						if (vertexIndex[0] < 0 && (maxVertexIndex == 0))
						{
							negative = true;
							maxVertexIndex = positions.size();
							maxUVIndex = texCoords.size();
							maxNormalIndex = normals.size();
						}

						//if only 2 index matches
						if (uvIndex[0] == 0 && matches == 2)
						{
							//ok in that case it's just positions and UVs
							if (!negative)
							{
								positionIndices.push_back(vertexIndex[0] - 1);
								positionIndices.push_back(vertexIndex[1] - 1);
								positionIndices.push_back(vertexIndex[2] - 1);

								uvIndices.push_back(uvIndex[0] - 1);
								uvIndices.push_back(uvIndex[1] - 1);
								uvIndices.push_back(uvIndex[2] - 1);
							}
						}

						else if ((matches % 3) == 0)
						{
							//all 3
							if (negative)
							{
								if (maxVertexRelativeIndex == 0)
								{
									maxVertexRelativeIndex = vertexIndex[0] + maxVertexIndex;
								}

								positionIndices.push_back(vertexIndex[0] + maxVertexIndex);
								positionIndices.push_back(vertexIndex[1] + maxVertexIndex);
								positionIndices.push_back(vertexIndex[2] + maxVertexIndex);
								positionIndices.push_back(vertexIndex[3] + maxVertexIndex);

								uvIndices.push_back(uvIndex[0] + maxUVIndex);
								uvIndices.push_back(uvIndex[1] + maxUVIndex);
								uvIndices.push_back(uvIndex[2] + maxUVIndex);
								uvIndices.push_back(uvIndex[3] + maxUVIndex);

								normalIndices.push_back(normalIndex[0] + maxNormalIndex);
								normalIndices.push_back(normalIndex[1] + maxNormalIndex);
								normalIndices.push_back(normalIndex[2] + maxNormalIndex);
								normalIndices.push_back(normalIndex[3] + maxNormalIndex);

								newMesh.tempVertices.push_back(positions[positionIndices[vertexIndex[0] + maxVertexIndex]]);
								newMesh.tempVertices.push_back(positions[positionIndices[vertexIndex[1] + maxVertexIndex]]);
								newMesh.tempVertices.push_back(positions[positionIndices[vertexIndex[2] + maxVertexIndex]]);
								newMesh.tempVertices.push_back(positions[positionIndices[vertexIndex[3] + maxVertexIndex]]);


								/*attributes[0].position = glm::vec4(positions[vertexIndex[0] + maxVertexIndex], 1);
								attributes[0].uv = texCoords[uvIndex[0] + maxUVIndex];
								attributes[0].normal = glm::vec4(normals[normalIndex[0] + maxNormalIndex], 1);

								attributes[1].position = glm::vec4(positions[vertexIndex[1] + maxVertexIndex], 1);
								attributes[1].uv = texCoords[uvIndex[1] + maxUVIndex];
								attributes[1].normal = glm::vec4(normals[normalIndex[1] + maxNormalIndex], 1);

								attributes[2].position = glm::vec4(positions[vertexIndex[2] + maxVertexIndex], 1);
								attributes[2].uv = texCoords[uvIndex[2] + maxUVIndex];
								attributes[2].normal = glm::vec4(normals[normalIndex[2] + maxNormalIndex], 1);

								attributes[3].position = glm::vec4(positions[vertexIndex[3] + maxVertexIndex], 1);
								attributes[3].uv = texCoords[uvIndex[3] + maxUVIndex];
								attributes[3].normal = glm::vec4(normals[normalIndex[3] + maxNormalIndex], 1);

								newMesh.vertices.push_back(attributes[0]);
								newMesh.vertices.push_back(attributes[1]);
								newMesh.vertices.push_back(attributes[2]);
								newMesh.vertices.push_back(attributes[3]);*/

								newMesh.originalIndices.push_back(vertexIndex[0] + maxVertexIndex);
								newMesh.originalIndices.push_back(vertexIndex[1] + maxVertexIndex);
								newMesh.originalIndices.push_back(vertexIndex[2] + maxVertexIndex);
								newMesh.originalIndices.push_back(vertexIndex[3] + maxVertexIndex);




							}
						}
					}

					else if (strcmp(wholeLine, "f") != 0 && newMesh.originalIndices.size() > 0)
					{
						if (!strcmp(wholeLine, "g"))
						{
							nextMesh = true;
						}
						meshes.push_back(newMesh);

						test++;
						if (test == 25)
						{
							printf("test \n");
						}

						break;
					}
				}
				
			}

		}

		fclose(file);
	}

	void LoadIntoGL()
	{
		glGenVertexArrays(1, &vertexArrayHandle);
		glBindVertexArray(vertexArrayHandle);
		glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(bufferTypes), bufferTypes);

		glBindBuffer(gl_array_buffer, bufferTypes[POS_VB]);
		glBufferData(gl_array_buffer, sizeof(positions[0]) * positions.size(), &positions[0], gl_static_draw);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_array_buffer, bufferTypes[TEXCOORD_VB]);
		glBufferData(gl_array_buffer, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], gl_static_draw);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_array_buffer, bufferTypes[NORMAL_VB]);
		glBufferData(gl_array_buffer, sizeof(normals[0]) * normals.size(), &normals[0], gl_static_draw);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_element_array_buffer, bufferTypes[INDEX_BUFFER]);
		glBufferData(gl_element_array_buffer, sizeof(positionIndices[0]) * positionIndices.size(), &positionIndices[0], gl_static_draw);

		glBindVertexArray(0);
		glBindBuffer(gl_array_buffer, 0);
		glBindBuffer(gl_element_array_buffer, 0);
	}

	void InitMesh(OBJMesh* mesh)
	{
		size_t faceIndex = 0;
		//for every face

		//for every vertex in that face
		for (size_t vertexIndex = 0; vertexIndex < mesh->tempVertices.size(); vertexIndex += 4, faceIndex = 0, mesh->indices.reserve(mesh->indices.size() + 4), mesh->vertices.reserve(mesh->vertices.size() + 4))
		{
			for(size_t faceIndex = 0; faceIndex < 4; faceIndex++)
			{
					//fill in the vertex buffer and re-establish index buffer
				vertexAttribute_t newAttribute;
				newAttribute.position = glm::vec4(positions[faceIndex], 1);

				//mesh->vertices[vertexIndex] = positions[]

				mesh->indices[faceIndex] = vertexIndex;
			}
		}
	}

	void DefineBuffers()
	{
		for(size_t iter = 0; iter < meshes.size(); iter++)
		{
			InitMesh(&meshes[iter]);

			glGenBuffers(1, &meshes[iter].vertexBufferHandle);
			glGenBuffers(1, &meshes[iter].indexBufferHandle);
			glGenVertexArrays(1, &meshes[iter].vertexArrayHandle);

			glBindVertexArray(meshes[iter].vertexArrayHandle);
			glBindBuffer(gl_array_buffer, meshes[iter].vertexBufferHandle);
			glBufferData(gl_array_buffer, sizeof(vertexAttribute_t) * meshes[iter].vertices.size(), meshes[iter].vertices.data(), gl_static_draw);

			glBindBuffer(gl_element_array_buffer, meshes[iter].indexBufferHandle);
			glBufferData(gl_element_array_buffer, sizeof(unsigned int) * meshes[iter].indices.size(), meshes[iter].indices.data(), gl_static_draw);

			//might cause more issues than prevent
			unsigned int attribID = 0;

			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::position);
			glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::position);

			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 2, GL_FLOAT, GL_FALSE, vertexOffset::uv);
			glVertexAttribPointer(attribID, 2, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::uv);	
		}	
	}

	/*unsigned int AddVertGetIndex(std::vector<glm::vec3>& vertices, const glm::vec3& vertex)
	{
		auto oIter = std::find(vertices.begin(), vertices.end(), vertex);
		if (oIter != vertices.end())
		{
			return oIter - vertices.begin();
		}
		vertices.push_back(vertex);
		return vertices.size() - 1;
	}

	unsigned int GetIndexOfVert(std::vector<glm::vec3>& vertices, const glm::vec3& vertex)
	{
		auto oIter = std::find(vertices.begin(), vertices.end(), vertex);
		if (oIter != vertices.end())
		{
			return oIter - vertices.begin();
		}

		return -1;
	}

	static bool sortVertices(glm::vec3 a, glm::vec3 b)
	{
		return (glm::length2(a) > glm::length2(b));
	}

	void Get2Closest(const glm::vec3& origin, unsigned int& firstIndex, unsigned int& secondIndex)
	{
		//go through all vertices that are not origin and 
		auto oIter = std::find(positions.begin(), positions.end(), origin);
		if (oIter == positions.end())
		{
			//could not find matching vertex
			return;
		}

		glm::vec3 lowest = glm::vec3(99999);
		glm::vec3 secondLowest = glm::vec3(99999);

		for (auto iter : positions)
		{
			if (iter == origin)
			{
				continue;
			}

			//is current in loop closer than current?
			if (glm::length2(origin - iter) < glm::length2(origin - lowest))
			{
				lowest = iter;
				continue;
			}
			else if (glm::length2(origin - iter) < glm::length2(origin - secondLowest))
			{
				secondLowest = iter;
			}
		}

		firstIndex = GetIndexOfVert(positions, lowest);
		secondIndex = GetIndexOfVert(positions, secondLowest);
	}*/
};
