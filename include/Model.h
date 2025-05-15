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

	void AddBoneData(unsigned int BoneID, float Weight)
	{
		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
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

	model_t(const char* resourcePath = "models/SoulSpear/SoulSpear.fbx", bool ignoreCollision = false, bool keepData = false)
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

	glm::mat4 makeTransform()
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

		glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
		directory = resourcePath.substr(0, resourcePath.find_last_of('/'));
		
		ExtractNode(dataScene->nodes[0]);		

		glm::mat4 rootTransform = ConvertToGLM(dataScene->root_node->geometry_transform);
		globalInverse = glm::inverse(rootTransform);
	}

	void ExtractNode(ufbx_node* node)
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

			vertexAttribute_t attrib;
			std::vector<unsigned int> indicesVec(mesh->vertex_indices.begin(), mesh->vertex_indices.begin() + mesh->num_indices);

			int numTriangles = mesh->materials[0].num_triangles;

			if (mesh->vertex_position.exists)
			{
				int indicesPerVert = mesh->num_indices / mesh->vertices.count;

				for (size_t index = 0; index < mesh->num_faces; index++)
				{
					ufbx_face face = mesh->faces.data[index];
					std::vector<uint32_t> tri_indices(mesh->max_face_triangles * 3);
					//per face, triangulation needed :)
					auto numTris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);

					std::vector<uint32_t> indices(tri_indices.size());
					ufbx_vertex_stream stream = { tri_indices.data(), sizeof(glm::vec3) };
					auto numIndices = ufbx_generate_indices(&stream, 1, tri_indices.data(), tri_indices.size(), NULL, NULL);

					ufbx_face indface = mesh->faces[index];

					for (unsigned int tri_indice : tri_indices)
					{
						newMesh.indices.push_back(tri_indice);
					}

					for (size_t triIter = 0; triIter < numTris * 3; triIter++)
					{
						auto index = tri_indices[triIter];

						//position
						if (mesh->vertex_position.exists)
						{
							auto pos = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
							attrib.position = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
							
						}

						//normal
						if (mesh->vertex_normal.exists)
						{
							hasNormals = true;
							auto normal = mesh->vertex_normal.values.data[mesh->vertex_normal.indices.data[index]];
							attrib.normal = glm::vec4(normal.x, normal.y, normal.z, 1.0f);
						}

						//tangent
						if (mesh->vertex_tangent.exists)
						{
							hasTangentsAndBiTangents = true;
							auto tangent = mesh->vertex_tangent.values.data[mesh->vertex_tangent.indices.data[index]];
							attrib.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 1.0f);
						}

						//bitangent
						if (mesh->vertex_bitangent.exists)
						{
							hasTangentsAndBiTangents = true;
							auto biTangent = mesh->vertex_bitangent.values.data[mesh->vertex_bitangent.indices.data[index]];
							attrib.biNormal = glm::vec4(biTangent.x, biTangent.y, biTangent.z, 1.0f);
						}

						//uv
						if (mesh->vertex_uv.exists)
						{
							auto uv = mesh->vertex_uv.values.data[mesh->vertex_uv.indices.data[index]];
							attrib.uv = glm::vec2(uv.x, uv.y);
						}
						
						//color
						if (mesh->vertex_color.exists)
						{
							auto color = mesh->vertex_color.values.data[mesh->vertex_color.indices.data[index]];
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
		glBindBuffer(gl_array_buffer, 0);
		glBindBuffer(gl_element_array_buffer, 0);

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
		glBindBuffer(gl_array_buffer, mesh.vertexBufferHandle);
		glBufferData(gl_array_buffer, sizeof(vertexAttribute_t) * mesh.vertices.size(), mesh.vertices.data(), gl_static_draw);

		glBindBuffer(gl_element_array_buffer, mesh.indexBufferHandle);
		glBufferData(gl_element_array_buffer, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), gl_static_draw);

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


/*

void ExtractAnimations(const aiScene* scene)
{
	if (scene->HasAnimations())
	{
		for (size_t iter = 0; iter < scene->mNumAnimations; iter++)
		{
			animationNames.emplace(scene->mAnimations[iter]->mName.C_Str(), scene->mAnimations[iter]);
			ExtractAnimationNodes(scene->mAnimations[iter]);
		}
	}
}

void ExtractAnimationNodes(const aiAnimation* anim)
{
	for (size_t iter = 0; iter < anim->mNumChannels; iter++)
	{
		animNodeNames.emplace(anim->mChannels[iter]->mNodeName.C_Str(), anim->mChannels[iter]);
	}
}*/

/*std::vector<glm::vec4> GetMeshPosData(unsigned int meshID)
{
	std::vector<glm::vec4> posData;
	aiMesh* mesh = assimpScene->mMeshes[node->mMeshes[iter]];
	//if ignore collision is on, skip the node with the prefix UCX_
	std::string ue4String = "UCX_";
	std::string nodeName = mesh->mName.C_Str();
	bool isCollision = (nodeName.substr(0, 4).compare(ue4String) == 0);

	for (unsigned int vertexIter = 0; vertexIter < mesh->mNumVertices; vertexIter++)
	{
		if (mesh->mVertices != nullptr)
		{
			posData.push_back(glm::vec4(mesh->mVertices[vertexIter].x,
				mesh->mVertices[vertexIter].y, mesh->mVertices[vertexIter].z, 1.0f));
		}
	}
}*/

/*

//note this is BEFORE this mesh is added to the mesh vector (only works if all vertices are thrown in a single mesh)
void CalculateMeshVertexOffset(mesh_t& mesh, const std::vector<vertexAttribute_t>& verts, const aiMesh* assimpMesh)
{
	//if no meshes so far, set vtx and idx offsets to 0
	if(meshes.size() == 0)
	{
		mesh.vertexOffset = 0;
		mesh.indexOffset = 0;
	}
	else
	{
		//ok grab the vertex offset of the last mesh and add it's vertex list size
		mesh_t lastMesh = meshes[meshes.size() - 1];

		mesh.vertexOffset = numVertices;
		mesh.indexOffset = numIndices;
	}
	numVertices += assimpMesh->mNumVertices;
	numIndices += assimpMesh->mNumFaces * 3;
	mesh.numVertices = assimpMesh->mNumVertices;
	mesh.numIndices = assimpMesh->mNumFaces * 3;
}*/

/*void ExtractBoneOffsets(aiMesh* mesh, mesh_t& currentMesh, std::vector<vertexAttribute_t>& verts) //current mesh is if we move data to the mesh
{
	printf("mesh %s has %i bones \n", mesh->mName.C_Str(), mesh->mNumBones);
	//for each bone in the current mesh,
	for (size_t boneIter = 0; boneIter < mesh->mNumBones; boneIter++)
	{
		int boneIndex = 0;
		std::string boneName = mesh->mBones[boneIter]->mName.C_Str();

		//add bone name and bone offset to their containers
		if(boneLookup.count(boneName) == 0)
		{
			//boneIndex = numBones;
			//numBones++;
			boneLookup.emplace(boneName, 0);

			glm::mat4 boneTransform = ConvertToGLM(mesh->mBones[boneIter]->mOffsetMatrix);
			rawTransforms.push_back(boneTransform);
			boneLookup[boneName] = boneIndex;
		}
		else
		{
			boneIndex = boneLookup[boneName];
		}

		//for each bone referenced in this submesh
		printf("OGLDev: this mesh %s uses bone %s with %i weights \n", mesh->mName.C_Str(), mesh->mBones[boneIter]->mName.C_Str(), mesh->mBones[boneIter]->mNumWeights);
		for (size_t weightsIter = 0; weightsIter < mesh->mBones[boneIter]->mNumWeights; weightsIter++)
		{
			unsigned int boneID = boneLookup[boneName];
			aiVertexWeight weight = mesh->mBones[boneIter]->mWeights[weightsIter];
			UpdateBoneData(verts, weight, boneIndex, weight.mVertexId);
		}
	}
	boneBuffer.data.finalTransforms = std::vector<glm::mat4>(rawTransforms.size());
}

void UpdateBoneData(std::vector<vertexAttribute_t>& verts, aiVertexWeight& weight, unsigned int boneIndex, unsigned int vertexID)
{
	for (size_t iter = 0; iter < 4; iter++)
	{
		//for all 4 vertex attribute slots check if the current vertex weights are 0. if they are, fill in that data
		if (verts[vertexID].weight[iter] == 0.0f)
		{
			verts[vertexID].boneIndex[iter] = boneIndex;
			verts[vertexID].weight[iter] = weight.mWeight;
			return; //leave early to prevent the entire set of weights to be written to
		}
	}
}

void BindBoneTransforms(unsigned int meshID, unsigned int uniformSlot)
{
	//meshes[meshID].boneBuffer.BindToSlot(uniformSlot, gl_shader_storage_buffer);
}

*/

///////////////////////////////////////////////////////////////////////////////
/*
void LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	printf("mesh %s has %i bones \n", pMesh->mName.C_Str(), pMesh->mNumBones);
	for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			// Allocate an index for a new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = ConvertToGLM(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}
		printf("Mine: this mesh %s uses bone %s with %i weights \n", pMesh->mName.C_Str(), BoneName.c_str(), pMesh->mBones[i]->mNumWeights);
		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
			unsigned int VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;

			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}
}

void InitMesh(unsigned int MeshIndex,
	const aiMesh* paiMesh,
	std::vector<glm::vec3>& Positions,
	std::vector<glm::vec3>& Normals,
	std::vector<glm::vec2>& TexCoords,
	std::vector<VertexBoneData>& Bones,
	std::vector<unsigned int>& Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
	}

	LoadBones(MeshIndex, paiMesh, Bones);

	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
}

void InitFromScene(const aiScene* pScene, const std::string& Filename)
{
	m_Entries.resize(pScene->mNumMeshes);

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> TexCoords;
	std::vector<VertexBoneData> Bones;
	std::vector<unsigned int> Indices;

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	// Count the number of vertices and indices
	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex = NumVertices;
		m_Entries[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += m_Entries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Bones.resize(NumVertices);
	Indices.reserve(NumIndices);

	// Initialize the meshes in the scene one by one
	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
	}

	// Generate and populate the buffers with vertex attributes and the indices
/*
		glBindBuffer(gl_array_buffer, m_Buffers[POS_VB]);
		glBufferData(gl_array_buffer, sizeof(Positions[0]) * Positions.size(), &Positions[0], gl_static_draw);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_array_buffer, m_Buffers[TEXCOORD_VB]);
		glBufferData(gl_array_buffer, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], gl_static_draw);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_array_buffer, m_Buffers[NORMAL_VB]);
		glBufferData(gl_array_buffer, sizeof(Normals[0]) * Normals.size(), &Normals[0], gl_static_draw);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(gl_array_buffer, m_Buffers[BONE_VB]);
		glBufferData(gl_array_buffer, sizeof(Bones[0]) * Bones.size(), &Bones[0], gl_static_draw);
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

		glBindBuffer(gl_element_array_buffer, m_Buffers[INDEX_BUFFER]);
		glBufferData(gl_element_array_buffer, sizeof(Indices[0]) * Indices.size(), &Indices[0], gl_static_draw);
	}

	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}
		assert(0);

		return 0;
	}

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
		return 0;
	}

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
		return 0;
	}

	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumPositionKeys == 1) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}

		unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}

		unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
		unsigned int NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
		Out = Out.Normalize();
	}

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumScalingKeys == 1) {
			Out = pNodeAnim->mScalingKeys[0].mValue;
			return;
		}

		unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		unsigned int NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
	{
		glm::mat4 Identity;
		Identity = glm::identity<glm::mat4>();

		float TicksPerSecond = (float)(assimpScene->mAnimations[0]->mTicksPerSecond != 0 ? assimpScene->mAnimations[0]->mTicksPerSecond : 25.0f);
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, (float)assimpScene->mAnimations[0]->mDuration);

		ReadNodeHeirarchy(AnimationTime, assimpScene->mRootNode, Identity);

		Transforms.resize(m_NumBones);

		for (unsigned int i = 0; i < m_NumBones; i++) {
			Transforms[i] = m_BoneInfo[i].FinalTransformation;
		}
	}

	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
	{
		for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
			const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

			if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
				return pNodeAnim;
			}
		}

		return NULL;
	}

	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
	{
		std::string NodeName(pNode->mName.data);

		const aiAnimation* pAnimation = assimpScene->mAnimations[0];

		glm::mat4 NodeTransformation = ConvertToGLM(pNode->mTransformation);

		const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

		if (pNodeAnim) {
			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D Scaling;
			CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			glm::mat4 ScalingM;
			ScalingM = glm::scale(glm::identity<glm::mat4>(), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion RotationQ;
			CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
			glm::mat4 RotationM = glm::mat4(ConvertToGLM(RotationQ.GetMatrix()));

			// Interpolate translation and generate translation transformation matrix
			aiVector3D Translation;
			CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
			glm::mat4 TranslationM;
			TranslationM = glm::translate(glm::identity<glm::mat4>(), glm::vec3(Translation.x, Translation.y, Translation.z));

			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM * ScalingM;
		}

		glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

		if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
			unsigned int BoneIndex = m_BoneMapping[NodeName];
			m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
		}

		for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
		}
	}*/

	/*

	void Evaluate(std::string animationName, float seconds, bool loop, float framesPerSec, unsigned int animIndex = 0)
	{
		if (assimpScene->mNumAnimations > 0)
		{
			//get anim ticks per second
			float ticksPS = assimpScene->mAnimations[animIndex]->mTicksPerSecond;
			if (ticksPS == 0.f)
			{
				//set standard ticks per second to 24 for now
				ticksPS = 24.0f;
			}

			float animDuration = assimpScene->mAnimations[animIndex]->mDuration / framesPerSec;

			float tickTime = seconds * ticksPS;
			float animTimeInTicks = fmod(tickTime, assimpScene->mAnimations[animIndex]->mDuration); //duration is animation frames

			float frameTime = 0.0f;

			if (loop)
			{
				frameTime = glm::max<float>(fmod(seconds, animDuration), 0);
			}

			else
			{
				frameTime = glm::min<float>(glm::max<float>(seconds, 0), animDuration);
			}

			frameTime *= ticksPS;

			//this hack should allow us to skip ahead a bit
			if (skeletonFound)
			{
				SearchNodeHeirarchy(frameTime, skeletonNode, glm::identity<glm::mat4>());
			}
			else
			{
				SearchNodeHeirarchy(frameTime, assimpScene->mRootNode, glm::identity<glm::mat4>());
			}

			//for(auto iter : meshes)
			{
				//ok by now all of the transforms should have been processed into finalTransforms
				boneBuffer.Update(gl_shader_storage_buffer, gl_dynamic_draw,
					sizeof(glm::mat4) * boneBuffer.data.finalTransforms.size(),
					boneBuffer.data.finalTransforms.data());
			}
		}
	}

	void SearchNodeHeirarchy(float animationTime, aiNode* node, const glm::mat4& parentTransform, unsigned int animIndex = 0)
	{
		std::string nodeName = node->mName.C_Str();
		aiAnimation* anim = assimpScene->mAnimations[animIndex];

		glm::mat4 trans = ConvertToGLM(node->mTransformation);

		//need the aiNodeAnim from the current animation being accessed
		const aiNodeAnim* nodeAnimation = animNodeNames[nodeName];
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		if(nodeAnimation != nullptr)
		{
			glm::vec3 euler = glm::eulerAngles(rotation);
			CalcInterpolatePRS(position, rotation, scale, animationTime, nodeAnimation);

			glm::mat4 P = glm::translate(glm::identity<glm::mat4>(), position);
			glm::mat4 R = glm::toMat4(rotation);
			glm::mat4 S = glm::scale(glm::identity<glm::mat4>(), scale);

			//assimp matrix transformations go scale, rotation then translation
			//trans = S * R * P;
			trans = P * R * S;

			//looks like we found the root animation node. tag it to save performance later
			if(!skeletonFound)
			{
				skeletonNode = node;
				skeletonFound = true;
			}
		}

		glm::mat4 globalTransform = parentTransform * trans;

		//for each bone, look for all the meshes that use this bone and update the final transform of that bone
		//for (size_t meshIter = 0; meshIter < meshes.size(); meshIter++)
		{
			if (boneLookup.count(std::string(node->mName.C_Str())) > 0)
			{
				unsigned int boneIndex = boneLookup[node->mName.C_Str()];
				boneBuffer.data.finalTransforms[boneIndex] = globalInverse * globalTransform * rawTransforms[boneIndex];
			}
		}


		for(size_t nodeIter = 0; nodeIter < node->mNumChildren; nodeIter++)
		{
			SearchNodeHeirarchy(animationTime, node->mChildren[nodeIter], globalTransform);
		}
	}

	void CalcInterpolatePRS(glm::vec3& outPosition, glm::quat& outRotation, glm::vec3& outScale, float animationTime, const aiNodeAnim* nodeAnim)
	{

		bool shouldReturn = false;
		if(nodeAnim->mNumPositionKeys == 1)
		{
			outPosition = glm::vec3(nodeAnim->mPositionKeys[0].mValue.x, nodeAnim->mPositionKeys[0].mValue.y, nodeAnim->mPositionKeys[0].mValue.z);
			shouldReturn = true;
		}

		if (nodeAnim->mNumRotationKeys == 1)
		{
			outRotation = glm::quat(nodeAnim->mRotationKeys[0].mValue.x, nodeAnim->mRotationKeys[0].mValue.y, nodeAnim->mRotationKeys[0].mValue.z, nodeAnim->mRotationKeys[0].mValue.w);
			shouldReturn = true;
		}

		if (nodeAnim->mNumScalingKeys == 1)
		{
			outScale = glm::vec3(nodeAnim->mScalingKeys[0].mValue.x, nodeAnim->mScalingKeys[0].mValue.y, nodeAnim->mScalingKeys[0].mValue.z);
			shouldReturn = true;
		}

		if(shouldReturn)
		{
			return;
		}

		int positionIndex = GetPositionIndex(animationTime, nodeAnim);
		int rotationIndex = GetRotationIndex(animationTime, nodeAnim);
		int scaleIndex = GetScaleIndex(animationTime, nodeAnim);

		if (positionIndex > -1 && positionIndex + 1 < nodeAnim->mNumPositionKeys)
		{
			float deltaPositionTime = nodeAnim->mPositionKeys[positionIndex + 1].mTime - nodeAnim->mPositionKeys[positionIndex].mTime;
			float positionFactor = (animationTime - nodeAnim->mPositionKeys[positionIndex].mTime) / deltaPositionTime;
			assert(positionFactor >= 0.0f && positionFactor <= 1.0f);
			glm::vec3 startPosition = glm::vec3(nodeAnim->mPositionKeys[positionIndex].mValue.x, nodeAnim->mPositionKeys[positionIndex].mValue.y, nodeAnim->mPositionKeys[positionIndex].mValue.z);
			glm::vec3 endPosition = glm::vec3(nodeAnim->mPositionKeys[positionIndex + 1].mValue.x, nodeAnim->mPositionKeys[positionIndex + 1].mValue.y, nodeAnim->mPositionKeys[positionIndex + 1].mValue.z);

			glm::vec3 delta = endPosition - startPosition;
			outPosition = startPosition + positionFactor * delta;

			//does this need to be normalized?
			outPosition = glm::lerp(startPosition, endPosition, positionFactor);
		}

		if (rotationIndex > -1 && rotationIndex < nodeAnim->mNumRotationKeys + 1)
		{
			float deltaRotationTime = nodeAnim->mRotationKeys[rotationIndex + 1].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime;
			float rotationFactor = (animationTime - nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaRotationTime;
			assert(rotationFactor >= 0.0f && rotationFactor <= 1.0f);
			glm::quat startRotation = glm::quat(nodeAnim->mRotationKeys[rotationIndex].mValue.x, nodeAnim->mRotationKeys[rotationIndex].mValue.y, nodeAnim->mRotationKeys[rotationIndex].mValue.z, nodeAnim->mRotationKeys[rotationIndex].mValue.w);
			glm::quat endRotation = glm::quat(nodeAnim->mRotationKeys[rotationIndex + 1].mValue.x, nodeAnim->mRotationKeys[rotationIndex + 1].mValue.y, nodeAnim->mRotationKeys[rotationIndex + 1].mValue.z, nodeAnim->mRotationKeys[rotationIndex + 1].mValue.w);

			aiQuaternion outQuat;
			aiQuaternion::Interpolate(outQuat, nodeAnim->mRotationKeys[rotationIndex].mValue, nodeAnim->mRotationKeys[rotationIndex + 1].mValue, rotationFactor);
			outQuat = outQuat.Normalize();

			//i hope this works
			outRotation.x = outQuat.x;
			outRotation.y = outQuat.y;
			outRotation.z = outQuat.z;
			outRotation.w = outQuat.w;

			//does this need to be normalized?
			//outRotation = glm::lerp(startRotation, endRotation, rotationFactor);
		}

		if (scaleIndex > -1 && scaleIndex < nodeAnim->mNumScalingKeys + 1)
		{
			float deltaRotationTime = nodeAnim->mScalingKeys[scaleIndex + 1].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime;
			float scaleFactor = (animationTime - nodeAnim->mScalingKeys[scaleIndex].mTime) / deltaRotationTime;
			assert(scaleFactor >= 0.0f && scaleFactor <= 1.0f);
			glm::vec3 startScale = glm::vec3(nodeAnim->mScalingKeys[scaleIndex].mValue.x, nodeAnim->mScalingKeys[scaleIndex].mValue.y, nodeAnim->mScalingKeys[scaleIndex].mValue.z);
			glm::vec3 endScale = glm::vec3(nodeAnim->mScalingKeys[scaleIndex + 1].mValue.x, nodeAnim->mScalingKeys[scaleIndex + 1].mValue.y, nodeAnim->mScalingKeys[scaleIndex + 1].mValue.z);

			glm::vec3 delta = endScale - startScale;
			outScale = startScale + scaleFactor * delta;

			//does this need to be normalized?
			outScale = glm::lerp(startScale, endScale, scaleFactor);
		}
	}

	int GetPositionIndex(float animationTime, const aiNodeAnim* nodeAnim)
	{
		for (size_t iter = 0; iter < nodeAnim->mNumRotationKeys - 1; iter++)
		{
			//if the time of the current key is the same as the passed in key,
			if (animationTime < (float)nodeAnim->mPositionKeys[iter + 1].mTime)
			{
				return iter;
			}
		}
		return -1;
	}

	int GetRotationIndex(float animationTime, const aiNodeAnim* nodeAnim)
	{
		for (size_t iter = 0; iter < nodeAnim->mNumRotationKeys - 1; iter++)
		{
			//if the time of the current key is the same as the passed in key,
			if (animationTime < (float)nodeAnim->mRotationKeys[iter + 1].mTime)
			{
				return iter;
			}
		}
		return -1;
	}

	int GetScaleIndex(float animationTime, const aiNodeAnim* nodeAnim)
	{
		for(size_t iter = 0; iter < nodeAnim->mNumRotationKeys - 1; iter++)
		{
			//if the time of the current key is the same as the passed in key,
			if(animationTime < (float)nodeAnim->mScalingKeys[iter + 1].mTime)
			{
				return iter;
			}
		}
		return -1;
	}*/
