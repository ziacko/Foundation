#pragma once

//ok here we just need a basic system to load snaders via JSON
static void LoadShaderProgramsFromConfigFile(tsl::robin_map<std::string, shaderProgram_t>* outPrograms = nullptr )
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
                                TinyShaders::LoadShader(localShader, yyjson_get_str(shaderName), localPath, localType);
                            }

                            if (localShader.isCompiled == true)
                            {
                                localProgram.shaders.push_back(localShader);
                            }
                        }
                    }
                    //ok now lets put it all together
                    TinyShaders::BuildProgramFromShaders(localProgram, localProgram.name, localProgram.inputs, localProgram.outputs, localProgram.shaders);
                    outPrograms->emplace(localProgram.name, localProgram);
                }
            }
        }
    }
}
