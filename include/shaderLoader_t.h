#pragma once
	//need to change this to have outputs for uniforms, uniform blocks and shader storage blocks
	inline void ProcessInterfaces(const shaderProgram_t& shaderProgram, bufferHandler_t* bufferHandler)
	{
#pragma region uniforms
		// Enumerate standalone uniforms and print their actual locations
		GLint numUniforms = 0;
		glGetProgramInterfaceiv(shaderProgram.handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

		// Get uniform properties
		constexpr std::array<GLenum, 8> props = {
			GL_NAME_LENGTH, // Length of name
			GL_TYPE, // Data type
			GL_LOCATION, // Shader location
			GL_BLOCK_INDEX, // Block index if in block
			GL_ARRAY_SIZE, // Size if array
			GL_ARRAY_STRIDE, // Stride if array
			GL_MATRIX_STRIDE, // Matrix stride if matrix
			GL_IS_ROW_MAJOR, // Row major order if matrix,
		};
		GLint values[8];

		for (int i = 0; i < numUniforms; ++i) //only active uniforms are counted
		{
			uniform newUniform;

			glGetProgramResourceiv(shaderProgram.handle, GL_UNIFORM, i, 8, props.data(), 8, nullptr, values);

			// Skip uniforms that are part of a uniform block (they do not have locations)
			if (values[3] != -1)
				continue;

			std::vector<char> nameData(values[0]);
			glGetProgramResourceName(shaderProgram.handle, GL_UNIFORM, i, static_cast<GLsizei>(nameData.size()), nullptr, nameData.data());
			std::string name(nameData.begin(), nameData.end() - 1);

			//get uniform type
			auto type = typeLUT.at(values[1]);

			newUniform.name = name;
			newUniform.type = type;
			newUniform.location = values[2];
			newUniform.araySize = values[4];
			newUniform.arrayStride = values[5];
			newUniform.metrixStride = values[6];
			newUniform.isRowMajor = (values[7] == GL_TRUE);

			newUniform.handle = glGetUniformLocation(shaderProgram.handle, name.c_str());

			if (bufferHandler->uniforms.contains(name) == false)
			{
				//ok if this is a new uniform then create a new binding, use the current size of the container?
				if (newUniform.handle != -1)
				{
					auto size = bufferHandler->uniforms.size();
					glUniformBlockBinding(shaderProgram.handle, newUniform.handle, size);
					newUniform.bindingSlot = size;


					bufferHandler->uniforms.emplace(name, newUniform);
				}
			}

			else
			{
				//bind the block to the already existing slot
				if (newUniform.handle != -1)
				{
					glUniformBlockBinding(shaderProgram.handle, newUniform.handle, bufferHandler->uniforms[name].bindingSlot);

					//this should be fine i think...
				}
			}
		}

		// Enumerate uniform blocks and break down their members
		GLint numBlocks = 0;
		glGetProgramInterfaceiv(shaderProgram.handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);

		for (int b = 0; b < numBlocks; ++b)
		{
			reflectionBlock_t newBlock;
			// Query block name length, binding and total data size
			constexpr GLenum blockProps1[3] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE };
			GLint blockVals1[3] = { 0, 0, 0 };
			glGetProgramResourceiv(shaderProgram.handle, GL_UNIFORM_BLOCK, b, 3, blockProps1, 3, nullptr, blockVals1);

			std::vector<char> blockNameData(blockVals1[0]);
			glGetProgramResourceName(shaderProgram.handle, GL_UNIFORM_BLOCK, b, static_cast<GLsizei>(blockNameData.size()), nullptr, blockNameData.data());
			std::string blockName(blockNameData.begin(), blockNameData.end() - 1);

			newBlock.name = blockName;

			// Obtain the uniform block index first (uniform blocks don't have locations)
			newBlock.handle = static_cast<GLint>(glGetUniformBlockIndex(shaderProgram.handle, blockName.c_str()));

			//if this uniform block already exists, bind it to the existing slot and skip adding
			if (bufferHandler->uniformBlocks.contains(blockName) == true)
			{
				if (newBlock.handle != -1)
				{
					glUniformBlockBinding(shaderProgram.handle, static_cast<GLuint>(newBlock.handle), bufferHandler->uniformBlocks[blockName].bindingSlot);
				}
			}
			else
			{
				newBlock.bindingSlot = blockVals1[1];
				//newBlock.size = blockVals1[2];

				// Number of active variables in this block
				GLint numActive = 0;
				constexpr GLenum blockPropNum = GL_NUM_ACTIVE_VARIABLES;
				glGetProgramResourceiv(shaderProgram.handle, GL_UNIFORM_BLOCK, b, 1, &blockPropNum, 1, nullptr, &numActive);

				// Retrieve the indices of active uniform variables in this block (indices into GL_UNIFORM interface)
				std::vector<GLint> activeUniformIndices(std::max(0, numActive));
				if (numActive > 0)
				{
					constexpr GLenum blockPropActive = GL_ACTIVE_VARIABLES;
					glGetProgramResourceiv(shaderProgram.handle, GL_UNIFORM_BLOCK, b, 1, &blockPropActive, numActive, nullptr, activeUniformIndices.data());
				}

				newBlock.numActiveMembers = numActive;
				newBlock.blockType = reflectionBlock_t::type_e::uniform;

				if (newBlock.handle != -1)
				{
					// assign a new binding slot sequentially
					auto binding = static_cast<GLuint>(bufferHandler->uniformBlocks.size());
					glUniformBlockBinding(shaderProgram.handle, static_cast<GLuint>(newBlock.handle), binding);
					newBlock.bindingSlot = binding;

					newBlock.Initialize();

					bufferHandler->uniformBlocks.emplace(blockName, newBlock);
				}

				printf("Uniform Block %d: %s | binding=%d | size=%d bytes | members=%d\n", b, blockName.c_str(), blockVals1[1], blockVals1[2], numActive);

				// For each member uniform in the block, query detailed layout info
				for (int mi = 0; mi < numActive; ++mi)
				{
					uniform newUniform;
					int uIndex = activeUniformIndices[mi];

					// Get uniform properties
					glGetProgramResourceiv(shaderProgram.handle, GL_UNIFORM, uIndex, 8, props.data(), 8, nullptr, values);

					std::vector<char> nameData(values[0]);
					glGetProgramResourceName(shaderProgram.handle, GL_UNIFORM, uIndex, static_cast<GLsizei>(nameData.size()), nullptr, nameData.data());
					std::string name(nameData.begin(), nameData.end() - 1);

					//get uniform type
					auto type = typeLUT.at(values[1]);

					newUniform.name = name;
					newUniform.type = type;
					newUniform.location = values[2];
					newUniform.araySize = values[4];
					newUniform.arrayStride = values[5];
					newUniform.metrixStride = values[6];
					newUniform.isRowMajor = (values[7] == GL_TRUE);
				}
			}

			//bufferHandler.uniformBlocks.emplace()

			//ok now place the whole block in the list
		}

#pragma endregion

#pragma region storageBlock

		//next are shader blocks
		glGetProgramInterfaceiv(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
		for (uint16_t blockIter = 0; blockIter < numBlocks; ++blockIter)
		{
			reflectionBlock_t newBlock;
			// Query block name length, binding and total data size
			constexpr GLenum blockProps1[3] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE };
			GLint blockVals1[3] = { 0, 0, 0 };
			glGetProgramResourceiv(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, blockIter, 3, blockProps1, 3, nullptr, blockVals1);

			std::vector<char> blockNameData(blockVals1[0]);
			glGetProgramResourceName(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, blockIter, static_cast<GLsizei>(blockNameData.size()), nullptr, blockNameData.data());
			std::string blockName(blockNameData.begin(), blockNameData.end() - 1);

			newBlock.name = blockName;
			newBlock.bindingSlot = blockVals1[1];
			//newBlock.size = blockVals1[2];

			// Number of active variables in this block
			GLint numActive = 0;
			constexpr GLenum blockPropNum = GL_NUM_ACTIVE_VARIABLES;
			glGetProgramResourceiv(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, blockIter, 1, &blockPropNum, 1, nullptr, &numActive);

			// Retrieve the indices of active uniform variables in this block (indices into GL_UNIFORM interface)
			std::vector<GLint> activeUniformIndices(std::max(0, numActive));
			if (numActive > 0)
			{
				constexpr GLenum blockPropActive = GL_ACTIVE_VARIABLES;
				glGetProgramResourceiv(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, blockIter, 1, &blockPropActive, numActive, nullptr, activeUniformIndices.data());
			}

			newBlock.numActiveMembers = numActive;
			newBlock.blockType = reflectionBlock_t::type_e::storage;

			newBlock.handle = glGetProgramResourceIndex(shaderProgram.handle, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
			if (newBlock.handle != -1)
			{
				glShaderStorageBlockBinding(shaderProgram.handle, newBlock.handle, blockIter);
				newBlock.bindingSlot = blockIter;
			}

 			// For each member variable in the storage block, query detailed layout info
			for (int mi = 0; mi < numActive; ++mi)
			{
				uniform newUniform;
				int uIndex = activeUniformIndices[mi];

				// Active variable indices for SSBOs refer to the GL_BUFFER_VARIABLE interface
				constexpr std::array<GLenum, 6> bufferVarProps = {
					GL_NAME_LENGTH,
					GL_TYPE,
					GL_ARRAY_SIZE,
					GL_ARRAY_STRIDE,
					GL_MATRIX_STRIDE,
					GL_IS_ROW_MAJOR,
				};
				GLint bufferVarValues[6];

				// Get buffer variable properties
				glGetProgramResourceiv(shaderProgram.handle, GL_BUFFER_VARIABLE, uIndex, 6, bufferVarProps.data(), 6, nullptr, bufferVarValues);

				std::vector<char> nameData(bufferVarValues[0]);
				glGetProgramResourceName(shaderProgram.handle, GL_BUFFER_VARIABLE, uIndex, static_cast<GLsizei>(nameData.size()), nullptr, nameData.data());
				std::string name(nameData.begin(), nameData.end() - 1);

				//get variable type
				auto type = typeLUT.at(bufferVarValues[1]);

				newUniform.name = name;
				newUniform.type = type;
				newUniform.location = -1; // Buffer variables do not have locations
				newUniform.araySize = bufferVarValues[2];
				newUniform.arrayStride = bufferVarValues[3];
				newUniform.metrixStride = bufferVarValues[4];
				newUniform.isRowMajor = (bufferVarValues[5] == GL_TRUE);
			}
			newBlock.Initialize();
			bufferHandler->shaderStorageBlocks.emplace(blockName, newBlock);
		}
#pragma endregion
	}


//ok here we just need a basic system to load snaders via JSON
static void LoadShaderProgramsFromConfigFile(tsl::robin_map<std::string, shaderProgram_t>* outPrograms = nullptr, bufferHandler_t* bufferHandler = nullptr)
{
	auto currentDir = std::filesystem::current_path();
	std::vector<shader_t*> localShaders;

	auto workingDire = std::filesystem::current_path();
#if defined(DEBUG)
	printf("%s \n", workingDire.string().c_str());
#endif

	//add the two string together
	auto fileName = std::string(PROJECT_NAME) + ".json";
	auto shaderPathPart = workingDire / "assets/shaders/";

	auto fullPath = shaderPathPart / PROJECT_NAME / fileName.c_str();

	if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
	{
		//first load the json file from JSON into a string
		FILE* pConfigFile = fopen(fullPath.string().c_str(), "rb");
		if (!pConfigFile) {
#if defined(DEBUG)
			fprintf(stderr, "Failed to open JSON: %s\n", fullPath.string().c_str());
#endif
			return;
		}
		if (fseek(pConfigFile, 0, SEEK_END) != 0) { fclose(pConfigFile); return; }

		long fileSizeLong = ftell(pConfigFile);
		if (fileSizeLong <= 0) { fclose(pConfigFile); return; }
		const size_t fileSize = static_cast<size_t>(fileSizeLong);
		rewind(pConfigFile);

		std::string tempBuffer;
		tempBuffer.resize(fileSize);
		size_t bytesRead = fread(tempBuffer.data(), 1, fileSize, pConfigFile);
		fclose(pConfigFile);

		// Use exactly the bytes actually read
		tempBuffer.resize(bytesRead);

		//now the JSON part
		constexpr size_t kJsonFlags =
			YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS;

		yyjson_read_err err;
		yyjson_doc* jsonDoc = yyjson_read_opts(tempBuffer.data(), tempBuffer.size(), kJsonFlags, nullptr, &err);
		if (!jsonDoc) {
#if defined(DEBUG)
			fprintf(stderr, "JSON parse failed (%s) at pos %zu in %s\n",
					err.msg ? err.msg : "unknown", err.pos,
					fullPath.string().c_str());
#endif
			return;
		}

		yyjson_val* root = yyjson_doc_get_root(jsonDoc);
		assert(root != nullptr);

		if (yyjson_is_arr(root))
		{
			//if root is an array, get every member and break it down into parts
			yyjson_val* programArray = yyjson_arr_get(root, 0);
			if (programArray != nullptr)
			{
				uint8_t index, max;
				yyjson_val* currentItem;
				//max = yyjson_arr_size(programArray);
				//for every program
				yyjson_arr_foreach(root, index, max, currentItem)
				{
					shaderProgram_t localProgram;
					//now break it down per program

					//get name as string
					yyjson_val* name = yyjson_obj_get(currentItem, "name");
					if (name != nullptr && yyjson_is_str(name))
					{
						localProgram.name = yyjson_get_str(name);
					}
#if defined(DEBUG)
					printf("loading shader program: %s \n", localProgram.name.c_str());
#endif
					// get outputs
					yyjson_val* outputs = yyjson_obj_get(currentItem, "outputs");
					if (outputs != nullptr && yyjson_is_arr(outputs))
					{
						uint8_t outputIndex, outputMax = 0;
						yyjson_val* currentOutput;
						//for every output, grab the name
						yyjson_arr_foreach(outputs, outputIndex, outputMax, currentOutput)
						{
							std::string outputName = yyjson_get_str(currentOutput);
							if (outputName.empty() == false)
							{
								localProgram.outputs.emplace_back(outputName);
							}
						}
					}

					//get vertex attributes
					yyjson_val* vertAttributes = yyjson_obj_get(currentItem, "vertex attributes");
					if (vertAttributes != nullptr && yyjson_is_arr(vertAttributes))
					{
						uint8_t vertexIndex, vertMax = 0;
						yyjson_val* currentAttrib;
						yyjson_arr_foreach(vertAttributes, vertexIndex, vertMax, currentAttrib)
						{
							std::string attribName = yyjson_get_str(currentAttrib);
							if (attribName.empty() == false)
							{
								localProgram.inputs.emplace_back(attribName);
							}
						}
					}

					//ok, now for shaders. this is gonna be complicated :(
					yyjson_val* shaders = yyjson_obj_get(currentItem, "shaders");
					if (shaders != nullptr && yyjson_is_arr(shaders))
					{
						uint8_t shaderIndex, shaderMax = 0;
						yyjson_val* currentShader;
						yyjson_arr_foreach(shaders, shaderIndex, shaderMax, currentShader)
						{
							shader_t localShader;
							yyjson_val* shaderName = yyjson_obj_get(currentShader, "name");
							yyjson_val* shaderPath = yyjson_obj_get(currentShader, "path");
							yyjson_val* shaderType = yyjson_obj_get(currentShader, "type");
							if (shaderName != nullptr && yyjson_is_str(shaderName))
							if (shaderPath != nullptr && yyjson_is_str(shaderPath))
							if (shaderType != nullptr && yyjson_is_str(shaderType))
							{
#if defined(DEBUG)
								printf("loading shader: %s\n", yyjson_get_str(shaderName));
								printf("loading shader type: %s\n", yyjson_get_str(shaderType));
#endif
								const std::string newPath = std::string( yyjson_get_str(shaderPath));
								const std::string localPath = (shaderPathPart / PROJECT_NAME / newPath).string();
								shaderType_e localType = StringToShaderType(std::string(yyjson_get_str(shaderType)));

								//prepend the working directory to path
								ts::LoadShader(localShader, yyjson_get_str(shaderName), localPath, localType);
							}

							if (localShader.isCompiled == true)
							{
								localProgram.shaders.push_back(localShader);
							}
						}
					}
					//ok now lets put it all together
					ts::BuildProgramFromShaders(localProgram, localProgram.name, localProgram.inputs, localProgram.outputs, localProgram.shaders);
					if (localProgram.isCompiled == true)
					{
						if (bufferHandler != nullptr) ProcessInterfaces(localProgram, bufferHandler);
						outPrograms->emplace(localProgram.name, localProgram);
					}
				}
			}
		}
	}
}


