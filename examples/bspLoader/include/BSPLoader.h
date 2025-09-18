#include "Scene3D.h"
#include <stdint.h>

#pragma pack(push, 1)
using boundbox_t = struct
{
	glm::vec3 min;
	glm::vec3 max;
};
#pragma pack(pop)

#pragma pack(push, 1)
using bboxShort_t = struct
{
	short min;
	short max;
};
#pragma pack(pop)

#pragma pack(push, 1)
using edge_t = struct 
{
	short vertex0;
	short vertex1;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct entry_t
{
	int32_t offset;
	int32_t size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct headerInfo_t
{
	int32_t version;
	entry_t entities;           // List of Entities.
	entry_t planes;             // Map Planes.
								 // numplanes = size/sizeof(plane_t)
	entry_t miptex;             // Wall Textures.
	entry_t vertices;           // Map Vertices.
								 // numvertices = size/sizeof(vertex_t)
	entry_t visilist;           // Leaves Visibility lists.
	entry_t nodes;              // BSP Nodes.
								 // numnodes = size/sizeof(node_t)
	entry_t texinfo;            // Texture Info for faces.
								 // numtexinfo = size/sizeof(texinfo_t)
	entry_t faces;              // Faces of each surface.
								 // numfaces = size/sizeof(face_t)
	entry_t lightmaps;          // Wall Light Maps.
	entry_t clipnodes;          // clip nodes, for Models.
								 // numclips = size/sizeof(clipnode_t)
	entry_t leaves;             // BSP Leaves.
								 // numlaves = size/sizeof(leaf_t)
	entry_t lface;              // List of Faces.
	entry_t edges;              // Edges of faces.
								 // numedges = Size/sizeof(edge_t)
	entry_t edgesList;             // List of Edges.
	entry_t models;             // List of Models.
};
#pragma pack(pop)

#pragma pack(push, 1)
using bspModel_t = struct 
{
	boundbox_t		bound;
	glm::vec3		origin;
	int32_t			node_id0;
	int32_t			node_id1;
	int32_t			node_1d2;
	int32_t			node_id3;
	int32_t			numLeafs;
	int32_t			faceID;
	int32_t			faceNum;
};
#pragma pack(pop)

#pragma pack(push, 1)
using surface_t = struct
{
	glm::vec3	vectorS;
	float		distS;
	glm::vec3	vectorT;
	float		distT;
	uint32_t	textureID;
	uint32_t	animated;
};
#pragma pack(pop)

#pragma pack(push, 1)
using face_t = struct
{
	unsigned short		planeID;
	unsigned short		side;
	int32_t				ledgeID;
	unsigned short		ledgeNum;
	unsigned short		texInfoID;
	unsigned char		typeLight;
	unsigned char		baseLight;
	unsigned char		light[2];
	int32_t				lightmap;
};
#pragma pack(pop)

#pragma pack(push, 1)
using mipHeader_t = struct
{
	int32_t numTex;
	int32_t offsets[4]; //supposed to be [numTex]
};
#pragma pack(pop)

#pragma pack(push, 1)
using mipTex_t = struct
{
	char			name[16];
	uint32_t	width;
	uint32_t	height;
	uint32_t	offset1;
	uint32_t	offset2;
	uint32_t	offset4;
	uint32_t	offset8;
};
#pragma pack(pop)

#pragma pack(push, 1)
using node_t = struct
{
	int32_t			planeID;
	unsigned short		front;
	unsigned short		back;
	bboxShort_t			box;
	short		faceID;
	unsigned short		faceNum;
};
#pragma pack(pop)

#pragma pack(push, 1)
using leaf_t = struct
{
	int32_t				type;
	int32_t				visList;
	bboxShort_t			bound;
	unsigned short		faceIndex;
	unsigned short		faceNum;
	unsigned char		soundWater;
	unsigned char		soundSky;
	unsigned char		soundSlime;
	unsigned char		soundLava;
};
#pragma pack(pop)

#pragma pack(push, 1)
using plane_t = struct
{
	glm::vec3		normal;
	float			distance;
	int32_t			type;
};
#pragma pack(pop)

#pragma pack(push, 1)
using clipNode_t = struct 
{
	uint32_t	planeEnum;
	short			front;
	short			back;
};
#pragma pack(pop)

struct pair_t
{
	pair_t*		next;
	char*		key;
	char*		value;
};

struct entity_t
{
	glm::vec3	origin;
	int32_t		brushID;
	int32_t		brushNum;
	pair_t*		pairs;
};

struct BSPPayload
{
	BSPPayload(headerInfo_t headerInfo)
	{
		//version = 0;
		clipNodes.resize(headerInfo.clipnodes.size / sizeof(clipNode_t));
		edges.resize(headerInfo.edges.size / sizeof(edge_t));
		entities.resize(headerInfo.entities.size / sizeof(entity_t));
		surfaceFaces.resize(headerInfo.faces.size / sizeof(face_t));
		leaves.resize(headerInfo.leaves.size / sizeof(leaf_t));
		listEdges.resize(headerInfo.edgesList.size / sizeof(short));
		lfaces.resize(headerInfo.lface.size / sizeof(face_t));
		lightmaps.resize(headerInfo.lightmaps.size / sizeof(unsigned char));
		mipTexes.resize(headerInfo.miptex.size / sizeof(mipTex_t));
		bspModels.resize(headerInfo.models.size / sizeof(bspModel_t));
		nodes.resize(headerInfo.nodes.size / sizeof(node_t));
		planes.resize(headerInfo.planes.size / sizeof(plane_t));
		surfaces.resize(headerInfo.texinfo.size / sizeof(surface_t));
		vertices.resize(headerInfo.vertices.size / sizeof(glm::vec3));
		visibilityList.resize(headerInfo.visilist.size / sizeof(unsigned char));
	}

	BSPPayload() {};

	std::vector<plane_t>			planes;
	std::vector<leaf_t>				leaves;
	std::vector<glm::vec3>			vertices;
	std::vector<node_t>				nodes;
	std::vector<surface_t>			surfaces;
	std::vector<face_t>				surfaceFaces;
	std::vector<clipNode_t>			clipNodes;
	std::vector<face_t>				lfaces;
	std::vector<short>				listEdges;
	std::vector<edge_t>				edges;
	std::vector<bspModel_t>			bspModels;
	std::vector<unsigned char>		lightmaps;
	std::vector<unsigned char>		visibilityList;
	std::vector<entity_t>			entities;
	std::vector<mipTex_t>			mipTexes;
};



class BSPData
{
public:

	BSPData()
	{
		version = 0;
	}

	BSPData(headerInfo_t headerInfo, FILE* file)
	{
		//payload = BSPPayload(headerInfo);


/*
		fseek(file, sizeof(headerInfo_t), SEEK_END);
		unsigned int totalFileSize = ftell(file);
		void* test;
		fseek(file, headerInfo.planes.offset, SEEK_SET);
		fread(&test, totalFileSize, 1, file);

		payload = reinterpret_cast<BSPPayload*>(test);*/
		//get total size for maybe later

		auto blarg = sizeof(headerInfo);
		clipNodes.resize(headerInfo.clipnodes.size / sizeof(clipNode_t));
		fseek(file, headerInfo.clipnodes.offset, SEEK_SET);
		fread(clipNodes.data(), headerInfo.clipnodes.size, 1, file);

		//edges
		edges.resize(headerInfo.edges.size / sizeof(edge_t));
		fseek(file, headerInfo.edges.offset, SEEK_SET);
		fread(edges.data(), headerInfo.edges.size, 1, file);

		//info.entities
		entities.resize(headerInfo.entities.size / sizeof(entity_t));
		fseek(file, headerInfo.entities.offset, SEEK_SET);
		fread(entities.data(), headerInfo.entities.size, 1, file);

		//info.faces
		surfaceFaces.resize(headerInfo.faces.size / sizeof(face_t));
		fseek(file, headerInfo.faces.offset, SEEK_SET);
		fread(surfaceFaces.data(), headerInfo.faces.size, 1, file);

		//info.leaves
		leaves.resize(headerInfo.leaves.size / sizeof(leaf_t));
		fseek(file, headerInfo.leaves.offset, SEEK_SET);
		fread(leaves.data(), headerInfo.leaves.size, 1, file);

		//info.ledges
		listEdges.resize(headerInfo.edgesList.size / sizeof(short));
		fseek(file, headerInfo.edgesList.offset, SEEK_SET);
		fread(listEdges.data(), headerInfo.edgesList.size, 1, file);

		//info.lface
		faces.resize(headerInfo.lface.size / sizeof(face_t));
		fseek(file, headerInfo.lface.offset, SEEK_SET);
		fread(faces.data(), headerInfo.lface.size, 1, file);

		//info.lightmaps
		lightmaps.resize(headerInfo.lightmaps.size / sizeof(unsigned char));
		fseek(file, headerInfo.lightmaps.offset, SEEK_SET);
		fread(lightmaps.data(), headerInfo.lightmaps.size, 1, file);

		//mip texes
		mipTexes.resize(headerInfo.miptex.size / sizeof(mipTex_t));
		fseek(file, headerInfo.miptex.offset, SEEK_SET);
		fread(mipTexes.data(), headerInfo.miptex.size, 1, file);

		//models
		bspModels.resize(headerInfo.models.size / sizeof(bspModel_t));
		fseek(file, headerInfo.models.offset, SEEK_SET);
		fread(bspModels.data(), headerInfo.models.size, 1, file);

		//nodes
		nodes.resize(headerInfo.nodes.size / sizeof(node_t));
		fseek(file, headerInfo.nodes.offset, SEEK_SET);
		fread(nodes.data(), headerInfo.nodes.size, 1, file);

		//info.planes
		planes.resize(headerInfo.planes.size / sizeof(plane_t));
		fseek(file, headerInfo.planes.offset, SEEK_SET);
		fread(planes.data(), headerInfo.planes.size, 1, file);

		//info.texinfo
		surfaces.resize(headerInfo.texinfo.size / sizeof(surface_t));
		fseek(file, headerInfo.texinfo.offset, SEEK_SET);
		fread(surfaces.data(), headerInfo.texinfo.size, 1, file);

		//info.version
		version = 0;
		fseek(file, 0, SEEK_SET);
		fread(&version, sizeof(int32_t), 1, file);

		//info.vertices
		vertices.resize(headerInfo.vertices.size / sizeof(glm::vec3));
		fseek(file, headerInfo.vertices.offset, SEEK_SET);
		fread(vertices.data(), headerInfo.vertices.size, 1, file);

		//info.visilist
		visibilityList.resize(headerInfo.visilist.size / sizeof(unsigned char));
		fseek(file, headerInfo.visilist.offset, SEEK_SET);
		fread(visibilityList.data(), headerInfo.visilist.size, 1, file);

		CreateMesh();
	}

	void CreateMesh()
	{
		levelMesh.name = "test";

		vertexAttribute_t first;
		vertexAttribute_t second;
		vertexAttribute_t prepend;
		
		for (const auto& modelIter : bspModels)
		{
			for (size_t faceIter = 0; faceIter < modelIter.faceID + modelIter.faceNum; faceIter++)
			{

/*
				int firstEdge = listEdges[surfaceFaces[faceIter].ledgeID];
				glm::vec4 firstVertex;
				if (firstEdge < 0)
				{
					firstVertex = glm::vec4(vertices[edges[-firstEdge].vertex1], 1.0f);
				}
				else
				{
					firstVertex = glm::vec4(vertices[edges[firstEdge].vertex0], 1.0f);
				}
				prepend.position = firstVertex;*/
				face_t face = surfaceFaces[faceIter];
				for (size_t ledgeIter = face.ledgeID; ledgeIter < face.ledgeID + face.ledgeNum; ledgeIter++)
				{
					int ledge = listEdges[ledgeIter];
					if (ledge < 0)
					{
						first.position = glm::vec4(vertices[edges[-ledge].vertex1], 1.0f);
						second.position = glm::vec4(vertices[edges[-ledge].vertex0], 1.0f);
					}
					else
					{
						first.position = glm::vec4(vertices[edges[ledge].vertex0], 1.0f);
						second.position = glm::vec4(vertices[edges[ledge].vertex1], 1.0f);
					}

					//levelMesh.vertices.push_back(prepend);
					levelMesh.vertices.push_back(first);
					levelMesh.vertices.push_back(second);					
				}
			}
		}
	}

	int32_t version;
	std::vector<clipNode_t>			clipNodes;
	std::vector<edge_t>				edges;
	std::vector<face_t>				surfaceFaces;
	std::vector<leaf_t>				leaves;
	std::vector<short>				listEdges;
	std::vector<face_t>				faces;
	std::vector<unsigned char>		lightmaps;
	std::vector<mipTex_t>			mipTexes;
	std::vector<bspModel_t>			bspModels;
	std::vector<node_t>				nodes;
	std::vector<plane_t>			planes;
	std::vector<surface_t>			surfaces;
	std::vector<glm::vec3>			vertices;
	std::vector<unsigned char>		visibilityList;
	std::vector<entity_t>			entities;


	BSPPayload* payload;
	mesh_t levelMesh;
};

class BSPScene : public scene3D
{
public:

	BSPScene(
		const char* windowName = "Ziyad Barakat's portfolio (BSP Model)",
		camera* texModelCamera = new camera(glm::vec2(1280, 720), 1.0f, camera::projection_t::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = "../../resources/shaders/BSPLoader/BSPLoader.txt",
		model_t* model = new model_t("../../resources/models/Blaze/Blaze.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{

	}

	~BSPScene() {};

	void Initialize() override
	{
		scene3D::Initialize();

		LoadBSP();

		//move raw vertex data into a vertex buffer for the luls
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &BSPBuffer);

		glBindVertexArray(VAO);
		glBindBuffer(gl_array_buffer, BSPBuffer);
		glBufferData(gl_array_buffer, sizeof(glm::vec4) * quakeData.levelMesh.vertices.size(), quakeData.levelMesh.vertices.data(), gl_static_draw);

		unsigned int attribID = 0;

		glEnableVertexAttribArray(attribID);
		glVertexAttribBinding(attribID, 0);
		glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::position);
		glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::position);

		glBindVertexArray(0);
		glBindBuffer(gl_array_buffer, 0);
	}


protected:

	int currentTexture = 0;
	std::vector<const char*> textureNames;
	BSPData quakeData;

	GLuint VAO{};
	GLuint BSPBuffer{};

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene::BuildGUI(window, io);
		ImGui::Begin("textures");
		ImGui::ListBox("loaded textures", &currentTexture, textureNames.data(), textureNames.size());
		ImGui::End();

		scene3D::DrawCameraStats();
	}

	void Draw() override
	{
		DrawMeshes();

		DrawGUI(windows[0]);

		windows[0]->SwapDrawBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void LoadBSP()
	{
		headerInfo_t info{};

		//just for the time being load the header data
		FILE* pfile = fopen("../../resources/models/BSPs/E1M1.bsp", "r");

		size_t blah = fread(&info, sizeof(headerInfo_t), 1, pfile);

		//get total size for maybe later
		fseek(pfile, sizeof(headerInfo_t), SEEK_END);
		unsigned int totalFileSize = ftell(pfile);

		quakeData = BSPData(info, pfile);

		fclose(pfile);
	}

	virtual void DrawMeshes()
	{
			//glBindBuffer(gl_element_array_buffer, iter.indexBufferHandle);
			glBindVertexArray(VAO);
			glUseProgram(this->programGLID);

			glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

			if (true)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawArrays(GL_LINES, 0, quakeData.levelMesh.vertices.size());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};
