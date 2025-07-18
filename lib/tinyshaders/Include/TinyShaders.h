//created by Ziyad Barakat 2015

#ifndef TINY_SHADERS_H
#define TINY_SHADERS_H

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <gl/GL.h>
//disable annoying warnings about unsafe stdio functions
#pragma  warning(disable: 4474)
#pragma  warning(disable: 4996)
//this automatically loads the OpenGL library if you are using Visual Studio
//comment this out if you have your own method 
//#pragma comment (lib, "opengl32.lib")
#endif

#if defined(__linux__) 
#include <GL/gl.h>
#endif

#include <utility>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <bitset>
#include <cmath>
#include <ranges>

namespace TinyShaders
{
	class tShader;
	class tShaderProgram;
	class shaderManager;
	using parseUniformBlockEvent_t = std::function<void(GLuint errorNumber, std::string errorMessage)>;

	inline std::string defaultProgramBinaryExtension = ".glbin";
	inline std::string defaultBinaryPath = "./Shaders/";

	enum class error_e
	{
		success,
		invalidString,
		invalidShaderProgramName,
		invalidShaderProgramIndex,
		invalidShaderName,
		invalidShaderIndex,
		invalidFilePath,
		shaderProgramNotFound,
		shaderNotFound,
		invalidShaderType,
		shaderLoadFailed,
		shaderProgramLoadFailed,
		shaderProgramLinkFailed,
		shaderAlreadyLoaded,
		shaderProgramAlreadyExists,
		invalidSourceFile,
		shaderCompileFailed,
		shaderProgramCompileFailed
	};

	typedef std::pair<const error_e,  const std::string> errorEntry;
	const std::unordered_map<const error_e, const std::string> errorLUT =
	{
		errorEntry(error_e::invalidString, "Error: invalid string"),
		errorEntry(error_e::shaderNotFound, "Error: shader not found"),
		errorEntry(error_e::invalidFilePath, "Error: invalid file path"),
		errorEntry(error_e::invalidShaderName, "Error: invalid shader name"),
		errorEntry(error_e::invalidShaderType, "Error: invalid shader type"),
		errorEntry(error_e::invalidShaderIndex, "Error: invalid shader index"),
		errorEntry(error_e::invalidSourceFile, "Error: source file is invalid"),
		errorEntry(error_e::shaderLoadFailed, "Error: shader has failed to load"),
		errorEntry(error_e::shaderProgramNotFound, "Error: shader program not found"),
		errorEntry(error_e::shaderProgramLoadFailed, "Error: shader program load failed"),
		errorEntry(error_e::shaderAlreadyLoaded, "Error: shader has already been loaded"),
		errorEntry(error_e::shaderCompileFailed, "Error: the shader has failed to compile"),
		errorEntry(error_e::invalidShaderProgramName, "Error: invalid shader program name"),
		errorEntry(error_e::shaderProgramLinkFailed, "Error: shader program linking failed"),
		errorEntry(error_e::invalidShaderProgramIndex, "Error: invalid shader program index"),
		errorEntry(error_e::shaderProgramAlreadyExists, "Error: shader program already exists"),
		errorEntry(error_e::shaderProgramCompileFailed, "Error: the shader program has failed to compile"),
	};
	using managerErrorEvent_t = std::function<void(const errorEntry& entry)>;
	using shaderErrorEvent_t  = std::function<void(const tShader* shader, const errorEntry& entry)>;
	using shaderProgramErrorEvent_t  = std::function<void(const tShaderProgram* shaderProgram, const errorEntry& entry)>;

	enum class shaderType_e
	{
		vertex = GL_VERTEX_SHADER,
		fragment = GL_FRAGMENT_SHADER,
		geometry = GL_GEOMETRY_SHADER,
		tessControl =  GL_TESS_CONTROL_SHADER,
		tessEval = GL_TESS_EVALUATION_SHADER,
		compute = GL_COMPUTE_SHADER,
		invalid = -1,
	};

	inline std::vector<GLenum> interfaces = { GL_UNIFORM, GL_UNIFORM_BLOCK, GL_ATOMIC_COUNTER_BUFFER, GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT,
		GL_TRANSFORM_FEEDBACK_VARYING, GL_BUFFER_VARIABLE, GL_SHADER_STORAGE_BLOCK, GL_TRANSFORM_FEEDBACK_BUFFER,
		GL_VERTEX_SUBROUTINE, GL_FRAGMENT_SUBROUTINE, GL_GEOMETRY_SUBROUTINE, GL_TESS_CONTROL_SUBROUTINE, GL_TESS_EVALUATION_SUBROUTINE, GL_COMPUTE_SUBROUTINE,
		GL_VERTEX_SUBROUTINE_UNIFORM, GL_FRAGMENT_SUBROUTINE_UNIFORM, GL_GEOMETRY_SUBROUTINE_UNIFORM, GL_TESS_CONTROL_SUBROUTINE_UNIFORM, GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, GL_COMPUTE_SUBROUTINE_UNIFORM
	};

	inline std::string TypeToString(const GLenum& type)
	{
		switch (type)
		{
			case GL_FLOAT: return "float";
			case GL_FLOAT_VEC2: return "vec2";
			case GL_FLOAT_VEC3: return "vec3";
			case GL_FLOAT_VEC4: return "vec4";
			case GL_DOUBLE: return "double";
			case GL_DOUBLE_VEC2: return "dvec2";
			case GL_DOUBLE_VEC3: return "dvec3";
			case GL_DOUBLE_VEC4: return "dvec4";
			case GL_INT: return "int";
			case GL_INT_VEC2: return "ivec2";
			case GL_INT_VEC3: return "ivec3";
			case GL_INT_VEC4: return "ivec4";
			case GL_UNSIGNED_INT: return "uint";
			case GL_UNSIGNED_INT_VEC2: return "uvec2";
			case GL_UNSIGNED_INT_VEC3: return "uvec3";
			case GL_UNSIGNED_INT_VEC4: return "uvec4";
			case GL_BOOL: return "bool";
			case GL_BOOL_VEC2: return "bvec2";
			case GL_BOOL_VEC3: return "bvec3";
			case GL_BOOL_VEC4: return "bvec4";
			case GL_FLOAT_MAT2: return "mat2";
			case GL_FLOAT_MAT3: return "mat3";
			case GL_FLOAT_MAT4: return "mat4";
			//case GL_FLOAT_MAT2X3: return "mat2x3";
			//case GL_FLOAT_MAT2X4: return "mat2x4";
			//case GL_FLOAT_MAT3X2: return "mat3x2";
			//case GL_FLOAT_MAT3X4: return "mat3x4";
			//case GL_FLOAT_MAT4X2: return "mat4x2";
			//case GL_FLOAT_MAT4X3: return "mat4x3";
			case GL_DOUBLE_MAT2: return "dmat2";
			case GL_DOUBLE_MAT3: return "dmat3";
			case GL_DOUBLE_MAT2X3: return "dmat2x3";
			case GL_DOUBLE_MAT2X4: return "dmat2x4";
			case GL_DOUBLE_MAT3X2: return "dmat3x2";
			case GL_DOUBLE_MAT3X4: return "dmat3x4";
			case GL_DOUBLE_MAT4X2: return "dmat4x2";
			case GL_DOUBLE_MAT4X3: return "dmat4x3";
			case GL_SAMPLER_1D: return "sampler1D";
			case GL_SAMPLER_2D: return "sampler2D";
			case GL_SAMPLER_3D: return "sampler3D";
			case GL_SAMPLER_CUBE: return "samplerCube";
			case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow";
			case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
			case GL_SAMPLER_1D_ARRAY: return "sampler1DArray";
			case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
			case GL_SAMPLER_CUBE_MAP_ARRAY: return "samplerCubeArray";
			case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow";
			case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
			case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS";
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
			case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
			case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW: return "samplerCubeArrayShadow";
			case GL_SAMPLER_BUFFER: return "samplerBuffer";
			case GL_SAMPLER_2D_RECT: return "sampler2DRect";
			case GL_SAMPLER_2D_RECT_SHADOW: return "sampler2DRectShadow";
			case GL_INT_SAMPLER_1D: return "isampler1D";
			case GL_INT_SAMPLER_2D: return "isampler2D";
			case GL_INT_SAMPLER_3D: return "isampler3D";
			case GL_INT_SAMPLER_CUBE: return "isamplerCube";
			case GL_INT_SAMPLER_1D_ARRAY: return "isampler1DArray";
			case GL_INT_SAMPLER_2D_ARRAY: return "isampler2DArray";
			case GL_INT_SAMPLER_CUBE_MAP_ARRAY: return "isamplerCubeArray";
			case GL_INT_SAMPLER_2D_MULTISAMPLE: return "isampler2DMS";
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray";
			case GL_INT_SAMPLER_BUFFER: return "isamplerBuffer";
			case GL_INT_SAMPLER_2D_RECT: return "isampler2DRect";
			case GL_UNSIGNED_INT_SAMPLER_1D: return "usampler1D";
			case GL_UNSIGNED_INT_SAMPLER_2D: return "usampler2D";
			case GL_UNSIGNED_INT_SAMPLER_3D: return "usampler3D";
			case GL_UNSIGNED_INT_SAMPLER_CUBE: return "usamplerCube";
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "usampler1DArray";
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return "usampler2DArray";
			case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY: return "usamplerCubeArray";
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return "usampler2DMS";
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usampler2DMSArray";
			case GL_UNSIGNED_INT_SAMPLER_BUFFER: return "usamplerBuffer";
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return "usampler2DRect";
			case GL_IMAGE_1D: return "image1D";
			case GL_IMAGE_2D: return "image2D";
			case GL_IMAGE_3D: return "image3D";
			case GL_IMAGE_2D_RECT: return "image2DRect";
			case GL_IMAGE_CUBE: return "imageCube";
			case GL_IMAGE_BUFFER: return "imageBuffer";
			case GL_IMAGE_1D_ARRAY: return "image1DArray";
			case GL_IMAGE_2D_ARRAY: return "image2DArray";
			case GL_IMAGE_CUBE_MAP_ARRAY: return "imageCubeArray";
			case GL_IMAGE_2D_MULTISAMPLE: return "image2DMS";
			case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "image2DMSArray";
			case GL_INT_IMAGE_1D: return "iimage1D";
			case GL_INT_IMAGE_2D: return "iimage2D";
			case GL_INT_IMAGE_3D: return "iimage3D";
			case GL_INT_IMAGE_2D_RECT: return "iimage2DRect";
			case GL_INT_IMAGE_CUBE: return "iimageCube";
			case GL_INT_IMAGE_BUFFER: return "iimageBuffer";
			case GL_INT_IMAGE_1D_ARRAY: return "iimage1DArray";
			case GL_INT_IMAGE_2D_ARRAY: return "iimage2DArray";
			case GL_INT_IMAGE_CUBE_MAP_ARRAY: return "iimageCubeArray";
			case GL_INT_IMAGE_2D_MULTISAMPLE: return "iimage2DMS";
			case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "iimage2DMSArray";
			case GL_UNSIGNED_INT_IMAGE_1D: return "uimage1D";
			case GL_UNSIGNED_INT_IMAGE_2D: return "uimage2D";
			case GL_UNSIGNED_INT_IMAGE_3D: return "uimage3D";
			case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "uimage2DRect";
			case GL_UNSIGNED_INT_IMAGE_CUBE: return "uimageCube";
			case GL_UNSIGNED_INT_IMAGE_BUFFER: return "uimageBuffer";
			case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return "uimage1DArray";
			case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return "uimage2DArray";
			case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY: return "uimageCubeArray";
			case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return "uimage2DMS";
			case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "uimage2DMSArray";
			case GL_UNSIGNED_INT_ATOMIC_COUNTER: return "atomic_uint";
			default:
			{
				return "none";
			}
		}
	}

	/*
	* a TShader is a wrapper for an OpenGL shader
	*/
	class tShader
	{
		friend class shaderManager;

	public:

		std::string			name;			/**< The name of the shader component */
		std::string			filePath;		/**< The FilePath of the component */
		std::string			buffer;			/**< keep a hold of the shader code for debugging */
		GLuint				handle;			/**< The handle to the shader in OpenGL */
		shaderType_e		type;			/**< The type of shader (Vertex, Fragment, etc.) */
		GLboolean			isCompiled;		/**< Whether the shader has been compiled */
		GLuint				pipelineHandle;
		GLboolean			separable;

		explicit tShader(std::string  shaderName, const shaderType_e& shaderType, std::string  shaderFilePath, const bool& separable = true) :
			name(std::move(shaderName)), filePath(std::move(shaderFilePath)), type(shaderType), isCompiled(false), separable(separable)
		{
			pipelineHandle = 0;
			handle = 0;
		}

		explicit tShader(std::string  shaderName, std::string  buffer, const shaderType_e& shaderType, const bool& separable = true)
			: name(std::move(shaderName)), buffer(std::move(buffer)), type(shaderType), separable(separable)
		{
			type = shaderType;
			pipelineHandle = 0;
			handle = 0;
			isCompiled = GL_FALSE;
			filePath = std::string("");
		}
		
		tShader() :
		handle(0), type(shaderType_e::vertex), isCompiled(false), pipelineHandle(0), separable(false) {}

		~tShader() = default;
	};

	/*
	* a tShaderProgram is a wrapper for an OpenGL shader program
	*/
	class tShaderProgram
	{
		friend class shaderManager;

	public:

		std::string						name;				/**< The name of the shader program */
		GLuint							handle;				/**< The OpenGL handle to the shader program */
		GLboolean						isCompiled;			/**< Whether the shader program has been linked successfully */
		std::vector< std::string >		inputs;				/**< The inputs of the shader program as a vector of strings */
		std::vector< std::string >		outputs;			/**< The outputs of the shader program as a vector of strings */
		std::vector< tShader >			shaders;			/**< The components that the shader program comprises as a vector */
		GLuint							pipelineID;			/**< The GL pipeline ID for building modular shader programs */


		/*
		* basic constructor
		*/
		tShaderProgram() : 
		handle(0), isCompiled(false), pipelineID(0) {}

		/*
		* uses the given values to create an OpenGL shader program
		*/
		tShaderProgram(std::string  programName,
			const std::vector< std::string >& programInputs,
			const std::vector< std::string >& programOutputs,
			const std::vector< tShader >& programShaders,
			bool saveBinary = false) :
			name(std::move(programName)), handle(0),
			inputs(programInputs), outputs(programOutputs),
			shaders(programShaders), pipelineID(0)
		{
			isCompiled = false;
		};

		/*
		* another bare-bones constructor
		*/
		explicit tShaderProgram(std::string  programName) :
			name(std::move(programName)), handle(0), isCompiled(false), pipelineID(0) {};

		tShaderProgram(std::string  programName, const GLuint programHandle) :
			name(std::move(programName)), handle(programHandle), isCompiled(false), pipelineID(0) {}

		tShaderProgram(std::string  programName, const tShader& computeShader)
			: name(std::move(programName)), handle(0), pipelineID(0)
		{
			shaders.push_back(computeShader);
			isCompiled = false;
		}

		~tShaderProgram() = default;
	};

	class shaderManager
	{
		public:

		parseUniformBlockEvent_t parseUniformBlockEvent;
		managerErrorEvent_t managerErrorEvent; /**< This is the callback to be used when a manager specific error has occurred */
		shaderErrorEvent_t shaderErrorEvent;
		shaderProgramErrorEvent_t shaderProgramErrorEvent;

		shaderManager() = default;
		~shaderManager() { Shutdown(); }

		tShader* GetShader(const std::string& name)
		{
			return shaders[name].get();
		}

		tShaderProgram* GetShaderProgram(const std::string& name)
		{
			return shaderPrograms[name].get();
		}

		/*
		* shuts down TinyShaders. deletes all OpenGL shaders and shader programs
		* as well as calling shutdown on all shader and programs and clears all vectors.
		*/
		void Shutdown()
		{
			for (auto & program : shaderPrograms | std::views::values)
			{
				ShutdownShaderProgram(program.get());
				const auto data = program.release();
				delete data;
			}

			for (auto& shader : shaders | std::views::values)
			{
				ShutdownShader(shader.get());
				const auto data = shader.release();
				delete data;
			}
			shaders.clear();
			shaderPrograms.clear();
		}

		/*
		* load an OpenGL shader
		*/
		void LoadShader( const std::string& name, const std::string& shaderFile, const shaderType_e& shaderType)
		{
			if (!name.empty())
			{
				std::unique_ptr<tShader> newShader(new tShader(name, shaderType, shaderFile));
				FileToBuffer(shaderFile, newShader->buffer);
				CompileShader(newShader.get());

				if (newShader->isCompiled)
				{
					shaders.emplace(name, std::move(newShader));
				}
				AddShaderErrorLog(newShader.get(), error_e::shaderCompileFailed);
			}
			AddErrorLog(error_e::invalidString);
		}

		/*
		* builds a new OpenGL shader program from already loaded shaders
		*/
		void BuildProgramFromShaders( const std::string& programName,
			const std::vector< std::string >& inputs,
			const std::vector< std::string >& outputs,
			const std::string& vertexShaderName,
			const std::string& fragmentShaderName,
			const std::string& geometryShaderName,
			const std::string& tessContShaderName,
			const std::string& tessEvalShaderName,
			const bool& saveBinary = false )
		{
			std::vector< tShader > shaderList;
			tShader vertexShader = *shaders[vertexShaderName].get();
			tShader fragmentShader = *shaders[fragmentShaderName].get();
			tShader geometryShader = *shaders[geometryShaderName].get();
			tShader tessControlShader = *shaders[tessContShaderName].get();
			tShader tessEvalShader = *shaders[tessEvalShaderName].get();

			shaderList.push_back( vertexShader );
			shaderList.push_back( fragmentShader );
			shaderList.push_back( geometryShader );
			shaderList.push_back( tessControlShader );
			shaderList.push_back( tessEvalShader );

			std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram( programName, inputs, outputs, shaderList, saveBinary ));
			CompileShaderProgram(newShaderProgram.get(), saveBinary);
			if (newShaderProgram->isCompiled)
			{
				shaderPrograms.emplace(programName, std::move(newShaderProgram));
			}
			AddShaderProgramErrorLog(newShaderProgram.get(), error_e::shaderProgramLoadFailed);
		}

		void BuildProgramFromShaders( const std::string& programName,
			const std::vector< std::string >& inputs,
			const std::vector< std::string >& outputs,
			const std::vector<tShader>& shaderList,
			const bool& saveBinary = false)
			{
				std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram( programName, inputs, outputs, shaderList, saveBinary ));
				CompileShaderProgram(newShaderProgram.get(), saveBinary);
				if (newShaderProgram->isCompiled)
				{
					shaderPrograms.emplace(programName, std::move(newShaderProgram));
				}
				else
				{
					AddShaderProgramErrorLog(newShaderProgram.get(), error_e::shaderProgramLoadFailed);
				}
			}

		void BuildProgramFromShaders(const std::string& shaderName,
			const std::string& computeShaderName,
			const bool saveBinary = false)
		{
			std::vector< tShader > shaderList;
			const tShader computeShader = *shaders[computeShaderName].get();

			shaderList.push_back(computeShader);
			std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram(shaderName, shaderList[0]));
			CompileShaderProgram(newShaderProgram.get(), saveBinary);
			if (newShaderProgram->isCompiled)
			{
				shaderPrograms.emplace(shaderName, std::move(newShaderProgram));
			}
			AddShaderProgramErrorLog(newShaderProgram.get(), error_e::shaderProgramLoadFailed);
		}

		void LoadShaderFromBuffer( const std::string& name, const std::string& buffer, const shaderType_e& shaderType )
		{
			if ( !buffer.empty() )
			{
				if ( name.empty() )
				{
					if (!shaders.contains(name))
					{
						std::unique_ptr<tShader> newShader(new tShader(name, buffer, shaderType));
						if (newShader->isCompiled)
						{
							shaders.emplace(name, std::move(newShader));
						}
						else
						{
							AddShaderErrorLog(newShader.get(), error_e::shaderCompileFailed);
						}
					}
					AddErrorLog(error_e::shaderAlreadyLoaded);
				}
				AddErrorLog(error_e::invalidShaderName);
			}
			AddErrorLog(error_e::invalidString);
		}

		private:

		std::vector<errorEntry> errorLog;
		std::unordered_map< std::string, std::unique_ptr<tShaderProgram>>	shaderPrograms;		/**< All loaded shader programs */
		std::unordered_map< std::string, std::unique_ptr<tShader>>			shaders;			/**< All loaded shaders*/

		/*
		* compile the shader from a given text file
		*/
		void CompileShader(tShader* shader)
			{
				//if the component hasn't been compiled yet
				if (!shader->isCompiled)
				{
					char errorLog[512];
					GLint successful;

					if (!shader->buffer.empty())
					{
						shader->handle = glCreateShader(static_cast<unsigned int>(shader->type));
						const char* str = shader->buffer.c_str();
						glShaderSource(shader->handle, 1, (const char**)&str, nullptr);
						glCompileShader(shader->handle);

						glGetShaderiv(shader->handle, GL_COMPILE_STATUS, &successful);
						glGetShaderInfoLog(shader->handle, sizeof(errorLog), nullptr, errorLog);

						if (shader->separable)
						{
							shader->pipelineHandle = glCreateProgram();
							glAttachShader(shader->pipelineHandle, shader->handle);

							glProgramParameteri(shader->pipelineHandle, GL_PROGRAM_SEPARABLE, GL_TRUE);
							glLinkProgram(shader->pipelineHandle);
						}

						if (successful != GL_TRUE)
						{
							//AddShader
#if defined(DEBUG)
							printf("%s \n", errorLog);
#endif
							AddShaderErrorLog(shader, error_e::shaderCompileFailed);
						}

						else
						{
							//ProcessInterfaces();
							shader->isCompiled = GL_TRUE;
						}
					}
					else
					{
						AddShaderErrorLog(shader, error_e::invalidSourceFile);
					}
				}
				else
				{
					//either the file name doesn't exist or the component has already been loaded
					AddShaderErrorLog(shader, error_e::invalidFilePath);
				}
			}

		/*
		* compile the OpenGL shader program with the given information
		*/
		void CompileShaderProgram(tShaderProgram* program, const bool saveBinary = true)
		{
			program->handle = glCreateProgram();
			char errorLog[512];
			GLint successful = GL_FALSE;
			if (!program->isCompiled)
			{
				for (const auto& shader : program->shaders)
				{
					//if (shader != nullptr)
					{
						glAttachShader(program->handle, shader.handle);
					}
				}

				// specify vertex input attributes
				for (size_t i = 0; i < program->inputs.size(); ++i)
				{
					glBindAttribLocation(program->handle, (GLuint)i, program->inputs[i].c_str());
				}

				// specify pixel shader outputs
				for (size_t i = 0; i < program->outputs.size(); ++i)
				{
					glBindFragDataLocation(program->handle, (GLuint)i, program->outputs[i].c_str());
				}

				if (saveBinary)
				{
					glProgramParameteri(program->handle, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
				}

				glLinkProgram(program->handle);
				glGetProgramiv(program->handle, GL_LINK_STATUS, &successful);

				if (successful != 1)
				{
					glGetProgramInfoLog(program->handle, sizeof(errorLog), nullptr, errorLog);
#if defined(DEBUG)
					printf("%s \n", errorLog);
					AddShaderProgramErrorLog(program, error_e::shaderProgramCompileFailed);
#endif
				}

				program->isCompiled = GL_TRUE;
				//ProcessInterfaces(program);

				if (saveBinary)
				{
					GLint binarySize = 0;
					glGetProgramiv(program->handle, GL_PROGRAM_BINARY_LENGTH, &binarySize);

					auto* buffer = (void*)malloc(binarySize);

					if (buffer == nullptr)
					{
						AddShaderProgramErrorLog(program, error_e::shaderProgramCompileFailed);
						return;
					}

					GLenum binaryFormat = GL_NONE;

					glGetProgramBinary(program->handle, binarySize, nullptr, &binaryFormat, buffer);

					std::string path;

					path += defaultBinaryPath;
					path += program->name;
					path += defaultProgramBinaryExtension;

					FILE* file = fopen(path.c_str(), "wb");
					fprintf(file, "%s\n", program->name.c_str());
					fprintf(file, "%i\n", binarySize);
					fprintf(file, "%i\n", binaryFormat);
					fwrite(buffer, binarySize, 1, file);
					fclose(file);
					path.clear();
				}
			}
				AddShaderProgramErrorLog(program, error_e::shaderProgramAlreadyExists);
		}

		void AddErrorLog(error_e newError, const uint16_t& fileLine = __LINE__, const std::string& functionName = __FUNCTION__)
		{
			auto newString = errorLUT.at(newError);

			//add to string then send it along
			newString.append(" | in function: %s ");
			newString.append(functionName);
			newString.append("at line %i \n");
			newString.append(std::to_string(fileLine));

			const auto newEntry = errorEntry(newError, newString);

			errorLog.push_back(newEntry);

			if (managerErrorEvent != nullptr)
			{
				managerErrorEvent(newEntry);
			}
		}

		void AddShaderErrorLog(const tShader* shader, error_e newError, const uint16_t& fileLine = __LINE__, const std::string& functionName = __FUNCTION__)
		{
			auto newString = errorLUT.at(newError);

			//add to string then send it along
			newString.append(" | in function: %s ");
			newString.append(functionName);
			newString.append("at line %i \n");
			newString.append(std::to_string(fileLine));

			const auto newEntry = errorEntry(newError, newString);

			errorLog.push_back(newEntry);

			if (shaderErrorEvent != nullptr)
			{
				shaderErrorEvent(shader, newEntry);
			}
		}

		void AddShaderProgramErrorLog(const tShaderProgram* program, error_e newError, const uint16_t& fileLine = __LINE__, const std::string& functionName = __FUNCTION__)
		{
			auto newString = errorLUT.at(newError);

			//add to string then send it along
			newString.append(" | in function: %s ");
			newString.append(functionName);
			newString.append("at line %i \n");
			newString.append(std::to_string(fileLine));

			const auto newEntry = errorEntry(newError, newString);

			errorLog.push_back(newEntry);

			if (shaderProgramErrorEvent != nullptr)
			{
				shaderProgramErrorEvent(program, newEntry);
			}
		}

		void ProcessInterfaces(const tShader* shader) const
		{
			//get all interfaces and resources

			//uniforms
			GLint numResources = 0;
			GLint resource = 0;

			std::vector<GLenum> supportedInterfaces = {};

			for (auto interfaceIter : interfaces)
			{
				glGetProgramInterfaceiv(shader->pipelineHandle, interfaceIter, GL_ACTIVE_RESOURCES, &numResources);

				std::vector<GLchar> nameData(256);
				std::vector<GLenum> properties;
				properties.push_back(GL_NAME_LENGTH);
				properties.push_back(GL_TYPE);
				properties.push_back(GL_ARRAY_SIZE);
				properties.push_back(GL_LOCATION);


				std::vector<GLint> values(properties.size());
				for (GLenum iter = 0; iter < (GLenum)numResources; iter++)
				{
					//for each uniform grab num resources
					glGetProgramInterfaceiv(shader->pipelineHandle, interfaceIter, iter, &resource);

					GLint valueLength = 0;
					glGetProgramResourceiv(shader->pipelineHandle, interfaceIter, iter, properties.size(), &properties[0], values.size(), &valueLength, &values[0]);

					nameData.resize(values[0]);
					glGetProgramResourceName(shader->pipelineHandle, interfaceIter, iter, nameData.size(), NULL, &nameData[0]);
					std::string name((char*)&nameData[0], nameData.size() - 1);

					printf("index %d: %s %s @ location %d.\n", iter, TypeToString(values[1]).c_str(), name.c_str(), values[3]);
				}
			}
		}

		void ProcessInterfaces(const tShaderProgram* shaderProgram)
		{
			//get all interfaces and resources

			//uniforms
			GLint numResources = 0;
			GLint resource = 0;
			GLint maxNumResources = 0;
			GLint numActiveBlocks = 0;

			std::vector<GLenum> supportedInterfaces = {};

			for (auto interfaceIter : interfaces)
			{
				//glGetProgramInterfaceiv(handle, interfaceIter, gl_active_uniform_blocks, &numActiveBlocks);
				glGetProgramInterfaceiv(shaderProgram->handle, interfaceIter, GL_ACTIVE_RESOURCES, &numResources);
				const GLenum blockProperties[1] = { GL_NUM_ACTIVE_VARIABLES };
				const GLenum activeUnifProp[1] = { GL_ACTIVE_VARIABLES };
				const GLenum unifProperties[3] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

				for (int blockIter = 0; blockIter < numResources; blockIter++)
				{
					int numActiveUniforms = 0;
					glGetProgramResourceiv(shaderProgram->handle, GL_UNIFORM_BLOCK, blockIter, 1, blockProperties, 1, NULL, &numActiveUniforms);
					if (numActiveUniforms == 0)
					{
						return;
					}

					std::vector<int>	blockUniforms(numActiveUniforms);
					glGetProgramResourceiv(shaderProgram->handle, GL_UNIFORM_BLOCK, blockIter, 1, activeUnifProp, numActiveUniforms, NULL, &blockUniforms[0]);

					for (size_t uniformIter = 0; uniformIter < numActiveUniforms; uniformIter++)
					{
						int values[3];
						glGetProgramResourceiv(shaderProgram->handle, GL_UNIFORM, blockUniforms[uniformIter], 3, unifProperties, 3, NULL, values);

						std::vector<char> nameData(values[0]);
						glGetProgramResourceName(shaderProgram->handle, GL_UNIFORM, blockUniforms[uniformIter], nameData.size(), NULL, &nameData[0]);
						std::string name(nameData.begin(), nameData.end() - 1);
						printf("%s \n", name.c_str());
					}
				}
			}
		}

		/*
		* remove the shader from OpenGL
		*/
		static void ShutdownShader(tShader* shader)
		{
			glDeleteShader(shader->handle);
			shader->isCompiled = GL_FALSE;
		}

		/*
		* shut down the shader program. delete it from OpenGL
		*/
		static void ShutdownShaderProgram(const tShaderProgram* shaderProgram)
		{
			glDeleteProgram(shaderProgram->handle);
			//get every shader inside and shut it down
			for (auto shader : shaderProgram->shaders)
			{
				ShutdownShader(&shader);
			}
		}

		/*
		* convert the given file to a single dimension c-string buffer
		*/
		void FileToBuffer(const std::string& path, std::string& bufferToFill)
		{
			FILE* file = fopen(path.c_str(), "rt");

			if (file == nullptr)
			{
				AddErrorLog(error_e::invalidFilePath);
			}

			//get total byte in given file
			fseek(file, 0, SEEK_END);
			const GLuint FileLength = ftell(file);
			fseek(file, 0, SEEK_SET);

			//allocate a file buffer and read the contents of the file
			std::string buffer(FileLength, '\0');
			fread(&buffer[0], sizeof(char), FileLength, file);

			fclose(file);
			bufferToFill = buffer;
		}
	};

	/*
	* convert the given string to a shader type
	*/
	inline void StringToShaderType( const std::string& typeString, shaderType_e& shaderTypeOut )
	{
		if( !typeString.empty() )
		{
			if(typeString == "vertex")
			{
				shaderTypeOut = shaderType_e::vertex;
				return;
			}
			if(typeString == "fragment")
			{
				shaderTypeOut = shaderType_e::fragment;
				return;
			}
			if(typeString == "geometry")
			{
				shaderTypeOut = shaderType_e::geometry;
				return;
			}
			if(typeString == "tessellation_control")
			{
				shaderTypeOut = shaderType_e::tessControl;
				return;
			}
			if(typeString == "tessellation_evaluation")
			{
				shaderTypeOut = shaderType_e::tessControl;
				return;
			}
			if(typeString == "compute")
			{
				shaderTypeOut = shaderType_e::compute;
				return;
			}
			shaderTypeOut = shaderType_e::invalid;
		}
	}

	/*
	* convert the given shader type to a string
	*/
	inline std::string ShaderTypeToString( const shaderType_e& shaderType )
	{
		switch ( shaderType )
		{
			case shaderType_e::vertex: return "vertex";
			case shaderType_e::fragment: return "fragment";
			case shaderType_e::geometry: return "geometry";
			case shaderType_e::tessControl: return "tessellation Control";
			case shaderType_e::tessEval: return "tessellation Evaluation";
			case shaderType_e::compute: return "compute";
			default: return "invalidShaderType";
		}
	}
}
#endif
