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
		errorEntry(error_e::invalidShaderProgramName, "Error: invalid shader program name"),
		errorEntry(error_e::invalidShaderProgramIndex, "Error: invalid shader program index"),
		errorEntry(error_e::invalidShaderName, "Error: invalid shader name"),
		errorEntry(error_e::invalidShaderIndex, "Error: invalid shader index"),
		errorEntry(error_e::invalidFilePath, "Error: invalid file path"),
		errorEntry(error_e::shaderProgramNotFound, "Error: shader program not found"),
		errorEntry(error_e::shaderNotFound, "Error: shader not found"),
		errorEntry(error_e::invalidShaderType, "Error: invalid shader type"),
		errorEntry(error_e::shaderLoadFailed, "Error: shader has failed to load"),
		errorEntry(error_e::shaderProgramLoadFailed, "Error: shader program load failed"),
		errorEntry(error_e::shaderProgramLinkFailed, "Error: shader program linking failed"),
		errorEntry(error_e::shaderAlreadyLoaded, "Error: shader has already been loaded"),
		errorEntry(error_e::shaderProgramAlreadyExists, "Error: shader program already exists"),
		errorEntry(error_e::invalidSourceFile, "Error: source file is invalid"),
		errorEntry(error_e::shaderCompileFailed, "Error: the shader has failed to compile"),
		errorEntry(error_e::shaderProgramCompileFailed, "Error: the shader program has failed to compile"),
	};
	using managerErrorEvent_t = std::function<void(const errorEntry& entry)>;
	using shaderErrorEvent_t  = std::function<void(const tShader* shader, const errorEntry& entry)>;
	using shaderProgramErrorEvent_t  = std::function<void(const tShaderProgram* shaderProgram, const errorEntry& entry)>;

	enum class shaderType_e
	{
		vertex = gl_vertex_shader,
		fragment = gl_fragment_shader,
		geometry = gl_geometry_shader,
		tessControl =  gl_tess_control_shader,
		tessEval = gl_tess_evaluation_shader,
		compute = gl_compute_shader,
		invalid = -1,
	};

	inline std::vector<GLenum> interfaces = { gl_uniform, gl_uniform_block, gl_atomic_counter_buffer, gl_program_input, gl_program_output,
		gl_transform_feedback_varying, gl_buffer_variable, gl_shader_storage_block, gl_transform_feedback_buffer,
		gl_vertex_subroutine, gl_fragment_subroutine, gl_geometry_subroutine, gl_tess_control_subroutine, gl_tess_evaluation_subroutine, gl_compute_subroutine,
		gl_vertex_subroutine_uniform, gl_fragment_subroutine_uniform, gl_geometry_subroutine_uniform, gl_tess_control_subroutine_uniform, gl_tess_evaluation_subroutine_uniform, gl_compute_subroutine_uniform
	};

	inline std::string TypeToString(const GLenum& type)
	{
		switch (type)
		{
			case GL_FLOAT: return "float";
			case gl_float_vec2: return "vec2";
			case gl_float_vec3: return "vec3";
			case gl_float_vec4: return "vec4";
			case GL_DOUBLE: return "double";
			case gl_double_vec2: return "dvec2";
			case gl_double_vec3: return "dvec3";
			case gl_double_vec4: return "dvec4";
			case GL_INT: return "int";
			case gl_int_vec2: return "ivec2";
			case gl_int_vec3: return "ivec3";
			case gl_int_vec4: return "ivec4";
			case GL_UNSIGNED_INT: return "uint";
			case gl_unsigned_int_vec2: return "uvec2";
			case gl_unsigned_int_vec3: return "uvec3";
			case gl_unsigned_int_vec4: return "uvec4";
			case gl_bool: return "bool";
			case gl_bool_vec2: return "bvec2";
			case gl_bool_vec3: return "bvec3";
			case gl_bool_vec4: return "bvec4";
			case gl_float_mat2: return "mat2";
			case gl_float_mat3: return "mat3";
			case gl_float_mat4: return "mat4";
			case gl_float_mat2x3: return "mat2x3";
			case gl_float_mat2x4: return "mat2x4";
			case gl_float_mat3x2: return "mat3x2";
			case gl_float_mat3x4: return "mat3x4";
			case gl_float_mat4x2: return "mat4x2";
			case gl_float_mat4x3: return "mat4x3";
			case gl_double_mat2: return "dmat2";
			case gl_double_mat3: return "dmat3";
			case gl_double_mat2x3: return "dmat2x3";
			case gl_double_mat2x4: return "dmat2x4";
			case gl_double_mat3x2: return "dmat3x2";
			case gl_double_mat3x4: return "dmat3x4";
			case gl_double_mat4x2: return "dmat4x2";
			case gl_double_mat4x3: return "dmat4x3";
			case gl_sampler_1d: return "sampler1D";
			case gl_sampler_2d: return "sampler2D";
			case gl_sampler_3d: return "sampler3D";
			case gl_sampler_cube: return "samplerCube";
			case gl_sampler_1d_shadow: return "sampler1DShadow";
			case gl_sampler_2d_shadow: return "sampler2DShadow";
			case gl_sampler_1d_array: return "sampler1DArray";
			case gl_sampler_2d_array: return "sampler2DArray";
			case gl_sampler_cube_map_array: return "samplerCubeArray";
			case gl_sampler_1d_array_shadow: return "sampler1DArrayShadow";
			case gl_sampler_2d_array_shadow: return "sampler2DArrayShadow";
			case gl_sampler_2d_multisample: return "sampler2DMS";
			case gl_sampler_2d_multisample_array: return "sampler2DMSArray";
			case gl_sampler_cube_shadow: return "samplerCubeShadow";
			case gl_sampler_cube_map_array_shadow: return "samplerCubeArrayShadow";
			case gl_sampler_buffer: return "samplerBuffer";
			case gl_sampler_2d_rect: return "sampler2DRect";
			case gl_sampler_2d_rect_shadow: return "sampler2DRectShadow";
			case gl_int_sampler_1d: return "isampler1D";
			case gl_int_sampler_2d: return "isampler2D";
			case gl_int_sampler_3d: return "isampler3D";
			case gl_int_sampler_cube: return "isamplerCube";
			case gl_int_sampler_1d_array: return "isampler1DArray";
			case gl_int_sampler_2d_array: return "isampler2DArray";
			case gl_int_sampler_cube_map_array: return "isamplerCubeArray";
			case gl_int_sampler_2d_multisample: return "isampler2DMS";
			case gl_int_sampler_2d_multisample_array: return "isampler2DMSArray";
			case gl_int_sampler_buffer: return "isamplerBuffer";
			case gl_int_sampler_2d_rect: return "isampler2DRect";
			case gl_unsigned_int_sampler_1d: return "usampler1D";
			case gl_unsigned_int_sampler_2d: return "usampler2D";
			case gl_unsigned_int_sampler_3d: return "usampler3D";
			case gl_unsigned_int_sampler_cube: return "usamplerCube";
			case gl_unsigned_int_sampler_1d_array: return "usampler1DArray";
			case gl_unsigned_int_sampler_2d_array: return "usampler2DArray";
			case gl_unsigned_int_sampler_cube_map_array: return "usamplerCubeArray";
			case gl_unsigned_int_sampler_2d_multisample: return "usampler2DMS";
			case gl_unsigned_int_sampler_2d_multisample_array: return "usampler2DMSArray";
			case gl_unsigned_int_sampler_buffer: return "usamplerBuffer";
			case gl_unsigned_int_sampler_2d_rect: return "usampler2DRect";
			case gl_image_1d: return "image1D";
			case gl_image_2d: return "image2D";
			case gl_image_3d: return "image3D";
			case gl_image_2d_rect: return "image2DRect";
			case gl_image_cube: return "imageCube";
			case gl_image_buffer: return "imageBuffer";
			case gl_image_1d_array: return "image1DArray";
			case gl_image_2d_array: return "image2DArray";
			case gl_image_cube_map_array: return "imageCubeArray";
			case gl_image_2d_multisample: return "image2DMS";
			case gl_image_2d_multisample_array: return "image2DMSArray";
			case gl_int_image_1d: return "iimage1D";
			case gl_int_image_2d: return "iimage2D";
			case gl_int_image_3d: return "iimage3D";
			case gl_int_image_2d_rect: return "iimage2DRect";
			case gl_int_image_cube: return "iimageCube";
			case gl_int_image_buffer: return "iimageBuffer";
			case gl_int_image_1d_array: return "iimage1DArray";
			case gl_int_image_2d_array: return "iimage2DArray";
			case gl_int_image_cube_map_array: return "iimageCubeArray";
			case gl_int_image_2d_multisample: return "iimage2DMS";
			case gl_int_image_2d_multisample_array: return "iimage2DMSArray";
			case gl_unsigned_int_image_1d: return "uimage1D";
			case gl_unsigned_int_image_2d: return "uimage2D";
			case gl_unsigned_int_image_3d: return "uimage3D";
			case gl_unsigned_int_image_2d_rect: return "uimage2DRect";
			case gl_unsigned_int_image_cube: return "uimageCube";
			case gl_unsigned_int_image_buffer: return "uimageBuffer";
			case gl_unsigned_int_image_1d_array: return "uimage1DArray";
			case gl_unsigned_int_image_2d_array: return "uimage2DArray";
			case gl_unsigned_int_image_cube_map_array: return "uimageCubeArray";
			case gl_unsigned_int_image_2d_multisample: return "uimage2DMS";
			case gl_unsigned_int_image_2d_multisample_array: return "uimage2DMSArray";
			case gl_unsigned_int_atomic_counter: return "atomic_uint";
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

		explicit tShader(std::string  shaderName, const shaderType_e& shaderType, const std::string& shaderFilePath, const bool& separable = true) :
			name(std::move(shaderName)), filePath(shaderFilePath), type(shaderType), isCompiled(false), separable(separable)
		{
			pipelineHandle = 0;
			handle = 0;
		}

		explicit tShader(std::string  shaderName, const std::string& buffer, const shaderType_e& shaderType, const bool& separable = true)
			: name(std::move(shaderName)), buffer(buffer), type(shaderType), separable(separable)
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
		tShaderProgram(const std::string& programName,
			const std::vector< std::string >& programInputs,
			const std::vector< std::string >& programOutputs,
			const std::vector< tShader >& programShaders,
			bool saveBinary = false) :
			name(programName), handle(0),
			inputs(programInputs), outputs(programOutputs),
			shaders(programShaders), pipelineID(0)
		{
			isCompiled = false;
		};

		/*
		* another bare-bones constructor
		*/
		explicit tShaderProgram(const std::string& programName) :
			name(programName), handle(0), isCompiled(false), pipelineID(0) {};

		tShaderProgram(const std::string& programName, const GLuint programHandle) :
			name(programName), handle(programHandle), isCompiled(false), pipelineID(0) {}

		tShaderProgram(const std::string& programName, const tShader& computeShader)
			: name(programName), handle(0), pipelineID(0)
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

		std::unordered_map< std::string, std::unique_ptr<tShaderProgram>>	shaderPrograms;		/**< All loaded shader programs */
		std::unordered_map< std::string, std::unique_ptr<tShader>>			shaders;			/**< All loaded shaders*/

		shaderManager(){}
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
				FileToBuffer(shaderFile, newShader.get()->buffer);
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
				glGetProgramInterfaceiv(shaderProgram->handle, interfaceIter, gl_active_resources, &numResources);
				const GLenum blockProperties[1] = { gl_num_active_variables };
				const GLenum activeUnifProp[1] = { gl_active_variables };
				const GLenum unifProperties[3] = { gl_name_length, gl_type, gl_location };

				for (int blockIter = 0; blockIter < numResources; blockIter++)
				{
					int numActiveUniforms = 0;
					glGetProgramResourceiv(shaderProgram->handle, gl_uniform_block, blockIter, 1, blockProperties, 1, NULL, &numActiveUniforms);
					if (numActiveUniforms == 0)
					{
						return;
					}

					std::vector<int>	blockUniforms(numActiveUniforms);
					glGetProgramResourceiv(shaderProgram->handle, gl_uniform_block, blockIter, 1, activeUnifProp, numActiveUniforms, NULL, &blockUniforms[0]);

					for (size_t uniformIter = 0; uniformIter < numActiveUniforms; uniformIter++)
					{
						int values[3];
						glGetProgramResourceiv(shaderProgram->handle, gl_uniform, blockUniforms[uniformIter], 3, unifProperties, 3, NULL, values);

						std::vector<char> nameData(values[0]);
						glGetProgramResourceName(shaderProgram->handle, gl_uniform, blockUniforms[uniformIter], nameData.size(), NULL, &nameData[0]);
						std::string name(nameData.begin(), nameData.end() - 1);
						printf("%s \n", name.c_str());
					}
				}
			}
		}


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
						glShaderSource(shader->handle, 1, (const char**)&str, 0);
						glCompileShader(shader->handle);

						glGetShaderiv(shader->handle, gl_compile_status, &successful);
						glGetShaderInfoLog(shader->handle, sizeof(errorLog), 0, errorLog);

						if (shader->separable)
						{
							shader->pipelineHandle = glCreateProgram();
							glAttachShader(shader->pipelineHandle, shader->handle);

							glProgramParameteri(shader->pipelineHandle, gl_program_separable, GL_TRUE);
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
		void CompileShaderProgram(tShaderProgram* program, bool saveBinary = true)
		{
			program->handle = glCreateProgram();
			char errorLog[512];
			GLint successful = GL_FALSE;
			if (!program->isCompiled)
			{
				for (auto& shader : program->shaders)
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
					glProgramParameteri(program->handle, gl_program_binary_retrievable_hint, GL_TRUE);
				}

				glLinkProgram(program->handle);
				glGetProgramiv(program->handle, gl_link_status, &successful);

				if (successful != 1)
				{
					glGetProgramInfoLog(program->handle, sizeof(errorLog), 0, errorLog);
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
					glGetProgramiv(program->handle, gl_program_binary_length, &binarySize);

					auto* buffer = (void*)malloc(binarySize);

					if (buffer == nullptr)
					{
						AddShaderProgramErrorLog(program, error_e::shaderProgramCompileFailed);
						return;
					}

					GLenum binaryFormat = GL_NONE;

					glGetProgramBinary(program->handle, binarySize, NULL, &binaryFormat, buffer);

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

			auto newEntry = errorEntry(newError, newString);

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

			auto newEntry = errorEntry(newError, newString);

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

			auto newEntry = errorEntry(newError, newString);

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
				glGetProgramInterfaceiv(shader->pipelineHandle, interfaceIter, gl_active_resources, &numResources);

				std::vector<GLchar> nameData(256);
				std::vector<GLenum> properties;
				properties.push_back(gl_name_length);
				properties.push_back(gl_type);
				properties.push_back(gl_array_size);
				properties.push_back(gl_location);


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
		case shaderType_e::vertex:
			{
				return "vertex";
			}
		case shaderType_e::fragment:
			{
				return "fragment";
			}
		case shaderType_e::geometry:
			{
				return "geometry";
			}
		case shaderType_e::tessControl:
			{
				return "tessellation Control";
			}
		case shaderType_e::tessEval:
			{
				return "tessellation Evaluation";
			}
		case shaderType_e::compute:
			{
				return "compute";
			}
		default: return "invalidShaderType";
		}
	}
}
#endif
