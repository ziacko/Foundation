#pragma once
#include <filesystem>
#include "VertexAttribute.h"

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

enum VB_TYPES {
	INDEX_BUFFER,
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	BONE_VB,
	NUM_VBs
};

struct boneTransforms_t
{
	std::vector<glm::mat4> finalTransforms;
};

struct mesh_t
{
	std::string								name;

	std::vector<vertexAttribute_t>			vertices;
	std::vector<unsigned int>				indices;
	std::vector<texture>					textures;

	glm::vec4								diffuse;
	glm::vec4								specular;
	glm::vec4								ambient;
	glm::vec4								emissive;
	glm::vec4								reflective;

	unsigned int							vertexArrayHandle;
	unsigned int							vertexBufferHandle;
	unsigned int							indexBufferHandle;

	unsigned int							numBones;

	bool									isCollision;

	unsigned int							vertexOffset;
	unsigned int							indexOffset;

	unsigned int							numVertices;
	unsigned int							numIndices;

	mesh_t()
	{
		textures = std::vector<texture>();

		diffuse = glm::vec4(0);
		specular = glm::vec4(0);
		ambient = glm::vec4(0);
		emissive = glm::vec4(0);
		reflective = glm::vec4(0);

		vertexArrayHandle = 0;
		isCollision = false;
		vertexOffset = 0;
		indexOffset = 0;
	}

	mesh_t(std::vector<vertexAttribute_t> inVertices, std::vector<unsigned int> inIndices, std::vector<texture> inTextures) : 
		textures(inTextures)
	{
		diffuse = glm::vec4(0);
		specular = glm::vec4(0);
		ambient = glm::vec4(0);
		emissive = glm::vec4(0);
		reflective = glm::vec4(0);

		vertexArrayHandle = 0;
		isCollision = false;
		vertexOffset = 0;
		indexOffset = 0;
	}
};

#define NUM_BONES_PER_VEREX 4
struct BoneInfo
{
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;

	BoneInfo()
	{
		BoneOffset = glm::mat4(0);
		FinalTransformation = glm::mat4(0);
	}
};

struct MeshEntry {
	MeshEntry()
	{
		NumIndices = 0;
		BaseVertex = 0;
		BaseIndex = 0;
		MaterialIndex = 0xFFFFFFFF;
	}

	unsigned int NumIndices;
	unsigned int BaseVertex;
	unsigned int BaseIndex;
	unsigned int MaterialIndex;
};

struct VertexBoneData
{
	unsigned int IDs[4];
	float Weights[4];

	VertexBoneData()
	{
		Reset();
	};

	void Reset()
	{
		ZERO_MEM(IDs);
		ZERO_MEM(Weights);
	}

	void AddBoneData(const uint32_t& BoneID, const float& Weight)
	{
		for (unsigned int i = 0; i < std::size(IDs); i++) {
			if (Weights[i] == 0.0) {
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}

		// should never get here - more bones than we have space for
		assert(0);
	}
};

class model_t
{
public:

	model_t(const char* resourcePath = "models/SoulSpear/SoulSpear.fbx", const bool& ignoreCollision = false, const bool& keepData = false)
	{
		this->resourcePath =  resourcePath;
		position = glm::vec3(0.0f, -2.0f, -3.0f);
		scale = glm::vec3(1.0f);
		rotation = glm::vec3(0.0f);
		this->ignoreCollision = ignoreCollision;
		isPicked = false;
		this->keepData = keepData;
		skeletonFound = false;
		skeletonID = 0;
		boneIndex = 0;
		//numBones = 0;
		hasBones = false;
		hasTangentsAndBiTangents = false;
		hasNormals = false;

		dataScene = nullptr;

		m_NumBones = 0;
	}

	glm::mat4 makeTransform() const
	{
		//make a rotation matrix
		glm::mat4 euler = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
		euler[3] = glm::vec4(position.x, position.y, position.z, 1.0f);
		euler = glm::scale(euler, scale);
			
		return euler;
	}

	void loadModel()
	{
		ufbx_load_opts opts = { 0 };
		opts.use_root_transform = true;
		opts.root_transform.rotation = ufbx_identity_quat;

		//opts.allow_missing_vertex_position = true;
		ufbx_error error;

		auto fullpath = ASSET_DIR + resourcePath;

		bool exists = std::filesystem::exists(fullpath);
		hasBones = false;
		assert(exists);

		dataScene = ufbx_load_file(fullpath.c_str(), &opts, &error);
		assert(dataScene != nullptr);
		
		m_GlobalInverseTransform = ConvertToGLM(dataScene->root_node->geometry_transform );
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(std::size(m_Buffers), m_Buffers);
		directory = resourcePath.substr(0, resourcePath.find_last_of('/'));
		
		ExtractNode(dataScene->nodes[0]);		

		glm::mat4 rootTransform = ConvertToGLM(dataScene->root_node->geometry_transform);
		globalInverse = glm::inverse(rootTransform);
	}

	void ExtractNode(const ufbx_node* node)
	{
		//extract mesh from this node

		if (node->is_root == false)
		{
			ExtractMesh(node->mesh);
		}

		//if the mesh has children, use recursion
		for (size_t iter = 0; iter < node->children.count; iter++)
		{
			ExtractNode(node->children[iter]);
		}
	}

	void ExtractMesh(ufbx_mesh* mesh)
	{
		mesh_t newMesh;
		newMesh.name = std::string(mesh->name.data, mesh->name.length);
		std::vector<vertexAttribute_t> verts;
		std::vector<texture> textures;

		//if ignore collision is on, skip the node with the prefix UCX_
		std::string ue4String = "UCX_";
		std::string nodeName = newMesh.name;
		newMesh.isCollision = (nodeName.substr(0, 4) == ue4String);
		std::vector<glm::vec4> positions;

		if (mesh->vertex_position.exists)
		{
			vertexAttribute_t attrib;
			//int indicesPerVert = mesh->num_indices / mesh->vertices.count;

			for (size_t index = 0; index < mesh->num_faces; index++)
			{
				ufbx_face face = mesh->faces.data[index];
				std::vector<uint32_t> tri_indices(mesh->max_face_triangles * 3);
				//per face, triangulation needed :)
				auto numTris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);

				std::vector<uint32_t> indices(tri_indices.size());
				//ufbx_vertex_stream stream = { tri_indices.data(), sizeof(glm::vec3) };
				//auto numIndices = ufbx_generate_indices(&stream, 1, tri_indices.data(), tri_indices.size(), NULL, NULL);

				//ufbx_face indface = mesh->faces[index];

				for (unsigned int tri_indice : tri_indices)
				{
					newMesh.indices.push_back(tri_indice);
				}

				for (size_t triIter = 0; triIter < numTris * 3; triIter++)
				{
					auto triIndex = tri_indices[triIter];

					//position
					if (mesh->vertex_position.exists)
					{
						auto pos = ufbx_get_vertex_vec3(&mesh->vertex_position, triIndex);
						attrib.position = glm::vec4(pos.x, pos.y, pos.z, 1.0f);

					}

					//normal
					if (mesh->vertex_normal.exists)
					{
						hasNormals = true;
						auto normal = mesh->vertex_normal.values.data[mesh->vertex_normal.indices.data[triIndex]];
						attrib.normal = glm::vec4(normal.x, normal.y, normal.z, 1.0f);
					}

					//tangent
					if (mesh->vertex_tangent.exists)
					{
						hasTangentsAndBiTangents = true;
						auto tangent = mesh->vertex_tangent.values.data[mesh->vertex_tangent.indices.data[triIndex]];
						attrib.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 1.0f);
					}

					//bitangent
					if (mesh->vertex_bitangent.exists)
					{
						hasTangentsAndBiTangents = true;
						auto biTangent = mesh->vertex_bitangent.values.data[mesh->vertex_bitangent.indices.data[triIndex]];
						attrib.biNormal = glm::vec4(biTangent.x, biTangent.y, biTangent.z, 1.0f);
					}

					//uv
					if (mesh->vertex_uv.exists)
					{
						auto uv = mesh->vertex_uv.values.data[mesh->vertex_uv.indices.data[triIndex]];
						attrib.uv = glm::vec2(uv.x, uv.y);
					}

					//color
					if (mesh->vertex_color.exists)
					{
						auto color = mesh->vertex_color.values.data[mesh->vertex_color.indices.data[triIndex]];
						attrib.color = glm::vec4(color.x, color.y, color.z, color.w);
					}

					positions.push_back(attrib.position);

					verts.push_back(attrib);
				}
			}
		}

		newMesh.vertices = verts;

		if(keepData)
		{
			posData.push_back(positions);
		}

		//for every material?
		for (size_t materialIter = 0; materialIter < mesh->materials.count; materialIter++)
		{

			ufbx_mesh_material mat = mesh->materials[materialIter];

			//grab diffuse data
			if (mat.material->fbx.diffuse_color.has_value)
			{
				auto diffuse = mat.material->fbx.diffuse_color.value_vec4;
				newMesh.diffuse = glm::vec4(diffuse.x, diffuse.y, diffuse.z, diffuse.w);

				//time to load associated textures
				if (mat.material->fbx.diffuse_color.texture_enabled && mat.material->fbx.diffuse_color.texture->has_file)
				{
					texture diffuseMap = loadMaterialTextures(mat.material->fbx.diffuse_color.texture, texture::textureType_t::diffuse, "diffuse");
					textures.insert(textures.end(), diffuseMap);
				}
			}

			//grab specular data
			if (mat.material->fbx.specular_color.has_value)
			{
				auto specular = mat.material->fbx.specular_color.value_vec4;
				newMesh.specular = glm::vec4(specular.x, specular.y, specular.z, specular.w);

				if (mat.material->fbx.specular_color.texture_enabled && mat.material->fbx.specular_color.texture->has_file)
				{
					texture specularMap = loadMaterialTextures(mat.material->fbx.specular_color.texture, texture::textureType_t::specular, "specular");
					textures.insert(textures.end(), specularMap);
				}
			}

			//grab normal data
			if (mat.material->fbx.normal_map.has_value)
			{
				if (mat.material->fbx.normal_map.texture_enabled && mat.material->fbx.normal_map.texture->has_file)
				{
					texture normalMap = loadMaterialTextures(mat.material->fbx.normal_map.texture, texture::textureType_t::normal, "normal");
					textures.insert(textures.end(), normalMap);
				}
			}

			//grab ambient data
			if (mat.material->fbx.ambient_color.has_value)
			{
				auto ambient = mat.material->fbx.ambient_color.value_vec4;
				newMesh.ambient = glm::vec4(ambient.x, ambient.y, ambient.z, ambient.w);

				if (mat.material->fbx.ambient_color.texture_enabled && mat.material->fbx.ambient_color.texture->has_file)
				{
					texture ambientMap = loadMaterialTextures(mat.material->fbx.ambient_color.texture, texture::textureType_t::image, "ambient");
					textures.insert(textures.end(), ambientMap);
				}
			}

			//emissive
			if (mat.material->fbx.emission_color.has_value)
			{
				auto emissive = mat.material->fbx.emission_color.value_vec4;
				newMesh.emissive = glm::vec4(emissive.x, emissive.y, emissive.z, emissive.w);

				/*if (mat.material->fbx.emission_color.texture->has_file)
				{
					texture ambientMap = loadMaterialTextures(mat.material->fbx.ambient_color.texture, texture::textureType_t::image, "ambient");
					textures.insert(textures.end(), ambientMap);
				}*/
			}

			//reflective
			if (mat.material->fbx.reflection_color.has_value)
			{
				auto reflection = mat.material->fbx.reflection_color.value_vec4;
				newMesh.reflective = glm::vec4(reflection.x, reflection.y, reflection.z, reflection.w);

			}

			//TODO add a fuckload more later!

		}

		LoadIntoGL(mesh, newMesh);

		newMesh.vertices = std::move(verts);
		newMesh.textures = std::move(textures);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		meshes.push_back(newMesh);
	}

	texture loadMaterialTextures(ufbx_texture* tex, texture::textureType_t inTexType, std::string uniformName)
	{
		texture outTex;

		std::string str =	std::string(tex->absolute_filename.data, tex->absolute_filename.length);
		const std::string& temp = str;

		std::string shorter = temp.substr(temp.find_last_of('/') + 1);
		std::string localPath = directory + '/' + shorter;

		texture newTex(localPath, inTexType, uniformName);
		newTex.LoadTexture();
		outTex = newTex;
		loadedTextures.push_back(newTex);

		return outTex;
	}

	void LoadIntoGL(ufbx_mesh* umesh, mesh_t& mesh)
	{
		glGenBuffers(1, &mesh.vertexBufferHandle);
		glGenBuffers(1, &mesh.indexBufferHandle);
		glGenVertexArrays(1, &mesh.vertexArrayHandle);

		glBindVertexArray(mesh.vertexArrayHandle);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribute_t) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

		//might cause more issues than prevent
		unsigned int attribID = 0;

		glEnableVertexAttribArray(attribID);
		glVertexAttribBinding(attribID, 0);
		glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::position);
		glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::position);

		if (hasNormals)
		{
			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::normal);
			glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::normal);
		}

		if (hasTangentsAndBiTangents)
		{
			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::tangent);
			glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::tangent);

			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::biNormal);
			glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::biNormal);
		}

		if (hasBones)
		{
			//if there are skeletal animations, load up the animation indices and weights
			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribIFormat(attribID, 4, GL_UNSIGNED_INT, vertexOffset::boneIndex);
			glVertexAttribIPointer(attribID++, 4, GL_UNSIGNED_INT, sizeof(vertexAttribute_t), (char*)vertexOffset::boneIndex);

			glEnableVertexAttribArray(attribID);
			glVertexAttribBinding(attribID, 0);
			glVertexAttribFormat(attribID, 4, GL_FLOAT, GL_FALSE, vertexOffset::weight);
			glVertexAttribPointer(attribID++, 4, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::weight);
		}

		glEnableVertexAttribArray(attribID);
		glVertexAttribBinding(attribID, 0);
		glVertexAttribFormat(attribID, 2, GL_FLOAT, GL_FALSE, vertexOffset::uv);
		glVertexAttribPointer(attribID, 2, GL_FLOAT, GL_FALSE, sizeof(vertexAttribute_t), (char*)vertexOffset::uv);
	}

	glm::mat4 ConvertToGLM(const ufbx_transform& uTrans)
	{
		//make a new transform out of this
		glm::mat4 outMat = glm::translate(glm::mat4(1.0f), glm::vec3(uTrans.translation.x, uTrans.translation.y, uTrans.translation.z));
		//outMat = glm::rotate(outMat, glm::degrees(0.0f), glm::vec3(uTrans.rotation.x, uTrans.rotation.y, uTrans.rotation.z)); //why is rotation failing?
		outMat = glm::scale(outMat, glm::vec3(uTrans.scale.x, uTrans.scale.y, uTrans.scale.z));

		return outMat;
	}

	void Render()
	{
		glBindVertexArray(m_VAO);

		for (unsigned int i = 0; i < m_Entries.size(); i++) {
			//const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

			//assert(MaterialIndex < m_Textures.size());

			/*if (m_Textures[MaterialIndex]) {
				m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
			}*/

			glDrawElementsBaseVertex(GL_TRIANGLES,
				m_Entries[i].NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex),
				m_Entries[i].BaseVertex);
		}

		// Make sure the VAO is not changed from the outside    
		glBindVertexArray(0);
	}

	std::string								resourcePath;
	std::vector<mesh_t>						meshes;
	std::string								directory;

	bool									isGUIActive;

	bool									skeletonFound;
	unsigned int							skeletonID;
	unsigned int							boneIndex;

	glm::vec3								position;
	glm::vec3								scale;
	glm::vec3								rotation;

	std::vector<texture>					loadedTextures;
	std::vector<std::vector<glm::vec4>>		posData;	

	std::vector<glm::mat4>					rawTransforms;
	std::map<std::string, unsigned int>		boneLookup;
	bufferHandler_t<boneTransforms_t>		boneBuffer;
	glm::mat4								globalInverse;

	bool									ignoreCollision;
	bool									isPicked;
	bool									keepData;

	bool									hasBones;
	bool									hasTangentsAndBiTangents;
	bool									hasNormals;

	//OGL Dev crap
	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;
	glm::mat4 m_GlobalInverseTransform;    
	std::vector<MeshEntry> m_Entries;

	GLuint m_VAO;
	GLuint m_Buffers[NUM_VBs];

	ufbx_scene*								dataScene;
};