//
// Created by ziyad on 1/04/25.
//

#ifndef SHADERLOADER_H
#define SHADERLOADER_H
#include <TinyShaders.h>

//ok here we just need a basuc system to load sahders via JSON

class shaderLoader_t
{
    //need to contain the shaders
    //need to have functions to load shaders via a config file

    public:
    shaderLoader_t() = default;

    ~shaderLoader_t()
    {
        //loadedPrograms.clear();
        loadedShaders.clear();
    }

    void LoadShaderProgramsFromConfigFile( const std::string& shaderConfigPath, bool saveBinary = false, std::vector< tShaderProgram>* outPrograms = nullptr )
    {
        auto currentDir = std::filesystem::current_path();
        uint16_t numInputs = 0;
        uint16_t numOutputs = 0;
        uint16_t numPrograms = 0;
        uint16_t numShaders = 0;
        uint16_t iterator;

        std::vector<std::string> inputs, outputs, paths, names;
        std::vector<tShader*> localShaders;

        auto workingDire = std::filesystem::current_path();
        printf("%s \n", workingDire.string().c_str());

        //add the two string together
        auto shaderPathPart = workingDire / "assets/shaders/";
        auto fullPath = workingDire / shaderConfigPath;

        if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
        {
            //first load the json file from JSON into a string
            FILE* pConfigFile = fopen(fullPath.string().c_str(), "r");
            fseek(pConfigFile, 0, SEEK_END);

            long fileSize = ftell(pConfigFile);

            fseek(pConfigFile, 0, SEEK_SET);

            std::string tempBuffer;
            tempBuffer.resize(fileSize);
            fread(&tempBuffer[0], fileSize, 1, pConfigFile);
            fclose(pConfigFile);

            //now the JSON part

            yyjson_doc* jsonDoc = yyjson_read(tempBuffer.c_str(), tempBuffer.size(), 0);
            assert(jsonDoc != nullptr);

            yyjson_val* root = yyjson_doc_get_root(jsonDoc);
            assert(root != nullptr);

            //start accessing values
                //program name
                //outputs - string - array
                //shaders - array
                    //string(name), string(path) , string(type)
                //vertex attributes
                    //string - array

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
                        tShaderProgram localProgram;
                        //now break it down per program

                        //get name as string
                        yyjson_val* name = yyjson_obj_get(currentItem, "name");
                        if (name != nullptr && yyjson_is_str(name))
                        {
                            localProgram.name = yyjson_get_str(name);
                        }

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
                                    // TODO: extend this later for types as well :)
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
                                    //TODO: also add types here
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
                                tShader localShader;
                                yyjson_val* shaderName = yyjson_obj_get(currentShader, "name");
                                yyjson_val* shaderPath = yyjson_obj_get(currentShader, "path");
                                yyjson_val* shaderType = yyjson_obj_get(currentShader, "type");
                                if (shaderName != nullptr && yyjson_is_str(shaderName))
                                if (shaderPath != nullptr && yyjson_is_str(shaderPath))
                                if (shaderType != nullptr && yyjson_is_str(shaderType))
                                {
                                    std::string newPath = std::string( yyjson_get_str(shaderPath));
                                    const std::string localPath = shaderPathPart.string().c_str() + newPath;
                                    TinyShaders::shaderType_t localType;
                                    TinyShaders::StringToShaderType(std::string(yyjson_get_str(shaderType)), localType);
                                    //prepend the working directory to path
                                    localShader = tShader(yyjson_get_str(shaderName), localType, localPath);
                                }

                                if (localShader.isCompiled == true)
                                {
                                    localProgram.shaders.push_back(localShader);
                                    loadedShaders.push_back(localShader);
                                }
                            }
                        }
                        //ok now lets put it all together
                        localProgram = tShaderProgram(localProgram.name, localProgram.inputs, localProgram.outputs, localProgram.shaders);

                        if (localProgram.isCompiled)
                        {
                            outPrograms->push_back(localProgram);
                        }
                    }
                }
            }
        }
    }

private:

    //absl::InlinedVector< tShaderProgram, 16 > loadedPrograms;
    absl::InlinedVector< tShader, 16 > loadedShaders;
    const std::string acceptedExt = ".json";
};

#endif //SHADERLOADER_H
