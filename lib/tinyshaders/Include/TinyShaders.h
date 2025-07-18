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

	inline std::string defaultProgramBinaryExtension = ".glbin";
	inline std::string defaultBinaryPath = "./Shaders/";

	namespace
	{
		enum class error_e
		{
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

		inline std::vector<GLenum> interfaces = { GL_UNIFORM, GL_UNIFORM_BLOCK, GL_ATOMIC_COUNTER_BUFFER, GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT,
			GL_TRANSFORM_FEEDBACK_VARYING, GL_BUFFER_VARIABLE, GL_SHADER_STORAGE_BLOCK, GL_TRANSFORM_FEEDBACK_BUFFER,
			GL_VERTEX_SUBROUTINE, GL_FRAGMENT_SUBROUTINE, GL_GEOMETRY_SUBROUTINE, GL_TESS_CONTROL_SUBROUTINE, GL_TESS_EVALUATION_SUBROUTINE, GL_COMPUTE_SUBROUTINE,
			GL_VERTEX_SUBROUTINE_UNIFORM, GL_FRAGMENT_SUBROUTINE_UNIFORM, GL_GEOMETRY_SUBROUTINE_UNIFORM, GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
			GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, GL_COMPUTE_SUBROUTINE_UNIFORM
		};

		typedef std::pair<uint32_t, const std::string> typeEntry;
		const std::unordered_map<uint32_t, const std::string> typeLUT =
		{
			typeEntry(GL_FLOAT, "float"),
			typeEntry(GL_FLOAT_VEC2, "vec2"),
			typeEntry(GL_FLOAT_VEC2,  "vec2"),
			typeEntry(GL_FLOAT_VEC3, "vec3"),
			typeEntry(GL_FLOAT_VEC4, "vec4"),
			typeEntry(GL_DOUBLE, "double"),
			typeEntry(GL_DOUBLE_VEC2, "dvec2"),
			typeEntry(GL_DOUBLE_VEC3, "dvec3"),
			typeEntry(GL_DOUBLE_VEC4, "dvec4"),
			typeEntry(GL_INT, "int"),
			typeEntry(GL_INT_VEC2, "ivec2"),
			typeEntry(GL_INT_VEC3, "ivec3"),
			typeEntry(GL_INT_VEC4, "ivec4"),
			typeEntry(GL_UNSIGNED_INT, "uint"),
			typeEntry(GL_UNSIGNED_INT_VEC2, "uvec2"),
			typeEntry(GL_UNSIGNED_INT_VEC3, "uvec3"),
			typeEntry(GL_UNSIGNED_INT_VEC4, "uvec4"),
			typeEntry(GL_BOOL, "bool"),
			typeEntry(GL_BOOL_VEC2, "bvec2"),
			typeEntry(GL_BOOL_VEC3, "bvec3"),
			typeEntry(GL_BOOL_VEC4, "bvec4"),
			typeEntry(GL_FLOAT_MAT2, "mat2"),
			typeEntry(GL_FLOAT_MAT3, "mat3"),
			typeEntry(GL_FLOAT_MAT4, "mat4"),
			typeEntry(GL_DOUBLE_MAT2,"dmat2"),
			typeEntry(GL_DOUBLE_MAT3,"dmat3"),
			typeEntry(GL_DOUBLE_MAT2X3,"dmat2x3"),
			typeEntry(GL_DOUBLE_MAT2X4,"dmat2x4"),
			typeEntry(GL_DOUBLE_MAT3X2,"dmat3x2"),
			typeEntry(GL_DOUBLE_MAT3X4,"dmat3x4"),
			typeEntry(GL_DOUBLE_MAT4X2,"dmat4x2"),
			typeEntry(GL_DOUBLE_MAT4X3,"dmat4x3"),
			typeEntry(GL_SAMPLER_1D,"sampler1D"),
			typeEntry(GL_SAMPLER_2D,"sampler2D"),
			typeEntry(GL_SAMPLER_3D,"sampler3D"),
			typeEntry(GL_SAMPLER_CUBE,"samplerCube"),
			typeEntry(GL_SAMPLER_1D_SHADOW,"sampler1DShadow"),
			typeEntry(GL_SAMPLER_2D_SHADOW,"sampler2DShadow"),
			typeEntry(GL_SAMPLER_1D_ARRAY,"sampler1DArray"),
			typeEntry(GL_SAMPLER_2D_ARRAY,"sampler2DArray"),
			typeEntry(GL_SAMPLER_CUBE_MAP_ARRAY,"samplerCubeArray"),
			typeEntry(GL_SAMPLER_1D_ARRAY_SHADOW,"sampler1DArrayShadow"),
			typeEntry(GL_SAMPLER_2D_ARRAY_SHADOW,"sampler2DArrayShadow"),
			typeEntry(GL_SAMPLER_2D_MULTISAMPLE,"sampler2DMS"),
			typeEntry(GL_SAMPLER_2D_MULTISAMPLE_ARRAY,"sampler2DMSArray"),
			typeEntry(GL_SAMPLER_CUBE_SHADOW,"samplerCubeShadow"),
			typeEntry(GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW,"samplerCubeArrayShadow"),
			typeEntry(GL_SAMPLER_BUFFER,"samplerBuffer"),
			typeEntry(GL_SAMPLER_2D_RECT,"sampler2DRect"),
			typeEntry(GL_SAMPLER_2D_RECT_SHADOW,"sampler2DRectShadow"),
			typeEntry(GL_INT_SAMPLER_1D,"isampler1D"),
			typeEntry(GL_INT_SAMPLER_2D,"isampler2D"),
			typeEntry(GL_INT_SAMPLER_3D,"isampler3D"),
			typeEntry(GL_INT_SAMPLER_CUBE,"isamplerCube"),
			typeEntry(GL_INT_SAMPLER_1D_ARRAY,"isampler1DArray"),
			typeEntry(GL_INT_SAMPLER_2D_ARRAY,"isampler2DArray"),
			typeEntry(GL_INT_SAMPLER_CUBE_MAP_ARRAY,"isamplerCubeArray"),
			typeEntry(GL_INT_SAMPLER_2D_MULTISAMPLE,"isampler2DMS"),
			typeEntry(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,"isampler2DMSArray"),
			typeEntry(GL_INT_SAMPLER_BUFFER,"isamplerBuffer"),
			typeEntry(GL_INT_SAMPLER_2D_RECT,"isampler2DRect"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_1D,"usampler1D"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_2D,"usampler2D"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_3D,"usampler3D"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_CUBE,"usamplerCube"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,"usampler1DArray"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,"usampler2DArray"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY,"usamplerCubeArray"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,"usampler2DMS"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,"usampler2DMSArray"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_BUFFER,"usamplerBuffer"),
			typeEntry(GL_UNSIGNED_INT_SAMPLER_2D_RECT,"usampler2DRect"),
			typeEntry(GL_IMAGE_1D,"image1D"),
			typeEntry(GL_IMAGE_2D,"image2D"),
			typeEntry(GL_IMAGE_3D,"image3D"),
			typeEntry(GL_IMAGE_2D_RECT,"image2DRect"),
			typeEntry(GL_IMAGE_CUBE,"imageCube"),
			typeEntry(GL_IMAGE_BUFFER,"imageBuffer"),
			typeEntry(GL_IMAGE_1D_ARRAY,"image1DArray"),
			typeEntry(GL_IMAGE_2D_ARRAY,"image2DArray"),
			typeEntry(GL_IMAGE_CUBE_MAP_ARRAY,"imageCubeArray"),
			typeEntry(GL_IMAGE_2D_MULTISAMPLE,"image2DMS"),
			typeEntry(GL_IMAGE_2D_MULTISAMPLE_ARRAY,"image2DMSArray"),
			typeEntry(GL_INT_IMAGE_1D,"iimage1D"),
			typeEntry(GL_INT_IMAGE_2D,"iimage2D"),
			typeEntry(GL_INT_IMAGE_3D,"iimage3D"),
			typeEntry(GL_INT_IMAGE_2D_RECT,"iimage2DRect"),
			typeEntry(GL_INT_IMAGE_CUBE,"iimageCube"),
			typeEntry(GL_INT_IMAGE_BUFFER,"iimageBuffer"),
			typeEntry(GL_INT_IMAGE_1D_ARRAY,"iimage1DArray"),
			typeEntry(GL_INT_IMAGE_2D_ARRAY,"iimage2DArray"),
			typeEntry(GL_INT_IMAGE_CUBE_MAP_ARRAY,"iimageCubeArray"),
			typeEntry(GL_INT_IMAGE_2D_MULTISAMPLE,"iimage2DMS"),
			typeEntry(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,"iimage2DMSArray"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_1D,"uimage1D"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_2D,"uimage2D"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_3D,"uimage3D"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_2D_RECT,"uimage2DRect"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_CUBE,"uimageCube"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_BUFFER,"uimageBuffer"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_1D_ARRAY,"uimage1DArray"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_2D_ARRAY,"uimage2DArray"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY,"uimageCubeArray"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,"uimage2DMS"),
			typeEntry(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,"uimage2DMSArray"),
			typeEntry(GL_UNSIGNED_INT_ATOMIC_COUNTER,"atomic_uint"),
		};

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

		typedef std::pair<const shaderType_e, const std::string> shaderTypeEntry;
		const std::unordered_map<const shaderType_e, const std::string> shaderTypeLUT =
		{
			shaderTypeEntry(shaderType_e::vertex, "vertex"),
			shaderTypeEntry(shaderType_e::fragment, "fragment"),
			shaderTypeEntry(shaderType_e::geometry, "geometry"),
			shaderTypeEntry(shaderType_e::tessControl, "tesselation control"),
			shaderTypeEntry(shaderType_e::tessEval, "tesselation evaluation"),
			shaderTypeEntry(shaderType_e::compute, "compute"),
			shaderTypeEntry(shaderType_e::invalid, "invalid"),
		};

		//lol this is some hack bullshit. lmao, even
		typedef std::pair<std::string, shaderType_e> shaderTypeEntryRev;
		const std::unordered_map<std::string, shaderType_e> shaderTypeRev =
		{
			shaderTypeEntryRev("vertex", shaderType_e::vertex),
			shaderTypeEntryRev("fragment", shaderType_e::fragment),
			shaderTypeEntryRev("geometry", shaderType_e::geometry),
			shaderTypeEntryRev("tesselation control", shaderType_e::tessControl),
			shaderTypeEntryRev("tesselation evaluation", shaderType_e::tessEval),
			shaderTypeEntryRev("compute", shaderType_e::compute),
			shaderTypeEntryRev("invalid", shaderType_e::invalid),
		};
	}

	using managerErrorEvent_t = std::function<void(const errorEntry& entry)>;
	using shaderErrorEvent_t  = std::function<void(const tShader* shader, const errorEntry& entry)>;
	using shaderProgramErrorEvent_t  = std::function<void(const tShaderProgram* shaderProgram, const errorEntry& entry)>;

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

		//parseUniformBlockEvent_t parseUniformBlockEvent;
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
		std::unordered_map< std::string, std::unique_ptr<tShader>>			shaders;			/**< All loaded shaders */

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

					printf("index %d: %s %s @ location %d.\n", iter, typeLUT.at(values[1]).c_str(), name.c_str(), values[3]);
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
	inline shaderType_e StringToShaderType(const std::string& typeString)
	{
		return shaderTypeRev.at(typeString);
	}

	/*
	* convert the given shader type to a string
	*/
	inline std::string ShaderTypeToString( const shaderType_e& shaderType )
	{
		return shaderTypeLUT.at(shaderType);
	}
}
#endif
