//created by Ziyad Barakat 2015

#ifndef TINY_SHADERS_H
#define TINY_SHADERS_H

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <gl/GL.h>
//disable annoying warnings about unsafe stdio functions
#pragma  warning(disable: 4474)
#pragma  warning(disable: 4996)
//this automatically loads the OpenGL library if you are using Visual studio 
//comment this out if you have your own method 
//#pragma comment (lib, "opengl32.lib")
#endif

#if defined(__linux__) 
#include <GL/gl.h>
#endif

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <memory>
#include <system_error>
#include <bitset>
#include <ctype.h>
#include <math.h>

namespace TinyShaders
{
	using parseUniformBlockEvent_t = std::function<void(GLuint errorNumber, std::string errorMessage)>;

	const int maxNumShaderComponents = 5;/**< The Maximum number of components a shader program can have. It's always 5*/
	std::string defaultProgramBinaryExtension = ".glbin";
	std::string defaultBinaryPath = "./Shaders/";
	std::string defaultBinaryconfigPath = "Binaries.txt";
	std::string defaultShaderProgramPath = "Shaders.txt";

	enum class error_t
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

	enum class shaderType_t
	{
		vertex = gl_vertex_shader,
		fragment = gl_fragment_shader,
		geometry = gl_geometry_shader,
		tessControl =  gl_tess_control_shader,
		tessEval = gl_tess_evaluation_shader,
		compute = gl_compute_shader
	};

	class errorCategory_t : public std::error_category
	{
		public:
			const char* name() const throw() override
			{
				return "tinyShaders";
			}

			virtual std::string message(int errorValue) const override
			{
				auto err = (error_t)errorValue;
				switch (err)
				{
				case error_t::success:
				{
					return "function call was successful \n";
				}
				case error_t::invalidString:
				{
					return "Error: string was invalid \n";
				}					
				case error_t::invalidShaderProgramName:
				{
					return "Error: invalid shader program name \n";
				}					
				case error_t::invalidShaderProgramIndex:
				{
					return "Error: invalid shader program index \n";
				}
				case error_t::invalidShaderName:
				{
					return "Error: invalid shader name \n";
				}
				case error_t::invalidShaderIndex:
				{
					return "Error: invalid shader index \n";
				}
				case error_t::invalidFilePath:
				{
					return "Error: invalid file path \n";
				}
				case error_t::shaderProgramNotFound:
				{
					return "Error: shader program not found \n";
				}
				case error_t::shaderNotFound:
				{
					return "Error: shader not found \n";
				}
				case error_t::invalidShaderType:
				{
					return "Error: invalid shader type \n";
				}
				case error_t::shaderLoadFailed:
				{
					return "Error: shader has failed to load \n";
				}
				case error_t::shaderProgramLoadFailed:
				{
					return "Error: shader program has failed to load \n";
				}
				case error_t::shaderProgramLinkFailed:
				{
					return "Error: shader program linking has failed \n";
				}
				case error_t::shaderAlreadyLoaded:
				{
					return "Error: shader has already been loaded. skipping \n";
				}
				case error_t::shaderProgramAlreadyExists:
				{
					return "Error: shader program has already been loaded. skipping \n";
				}
				case error_t::invalidSourceFile:
				{
					return "Error: source file is invalid \n";
				}

				case error_t::shaderCompileFailed:
				{
					return "Error: the shader has failed to compile \n";
				}

				case error_t::shaderProgramCompileFailed:
				{
					return "Error: the shader program has failed to compile \n";
				}

				default:
				{
					return "Unspecified error \n";
				}
				}
			}

			errorCategory_t() {};

			const static errorCategory_t& get()
			{
				const static errorCategory_t category;
				return category;
			}
	};

	inline std::error_code make_error_code(error_t errorCode)
	{
		return std::error_code(static_cast<int>(errorCode), errorCategory_t::get());
	}

	std::vector<GLenum> interfaces = { gl_uniform, gl_uniform_block, gl_atomic_counter_buffer, gl_program_input, gl_program_output,
		gl_transform_feedback_varying, gl_buffer_variable, gl_shader_storage_block, gl_transform_feedback_buffer,
		gl_vertex_subroutine, gl_fragment_subroutine, gl_geometry_subroutine, gl_tess_control_subroutine, gl_tess_evaluation_subroutine, gl_compute_subroutine,
		gl_vertex_subroutine_uniform, gl_fragment_subroutine_uniform, gl_geometry_subroutine_uniform, gl_tess_control_subroutine_uniform, gl_tess_evaluation_subroutine_uniform, gl_compute_subroutine_uniform
	};

	std::string typeToString(GLenum type)
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
}

namespace std
{
	template<> struct is_error_code_enum<TinyShaders::error_t> : std::true_type {};
};

namespace TinyShaders
{
	class tShader
	{
		friend class shaderManager;

	public:

		std::string			name;			/**<The name of the shader component */
		std::string			filePath;		/**<The FilePath of the component*/
		GLuint				handle;			/**<The handle to the shader in OpenGL*/
		shaderType_t		type;			/**<The type of shader ( Vertex, Fragment, etc.)*/
		GLboolean			isCompiled;		/**<Whether the shader has been compiled*/
		GLuint				pipelineHandle;
		GLboolean			seperable;

		//add reflection data
		//uniform buffers
		//shader storage buffers
		//textures
		//samplers
		//inputs and outputs
		//vertex attributes(name and type?)

		tShader(std::string shaderName, shaderType_t shaderType, std::string shaderFilePath, bool seperable = true) :
			name(shaderName), type(shaderType), isCompiled(false), filePath(shaderFilePath), seperable(seperable)
		{
			std::string buffer;
			pipelineHandle = 0;
			FileToBuffer(shaderFilePath, buffer);
			Compile(buffer);
		}

		tShader(std::string shaderName, std::string buffer, shaderType_t shaderType, bool seperable = true)
			: name(shaderName), type(shaderType), seperable(seperable)
		{
			type = shaderType;
			pipelineHandle = 0;
			isCompiled = GL_FALSE;
			Compile(buffer);
			filePath = std::string("");
		}
		
		tShader() :
		handle(0), type(shaderType_t::vertex), isCompiled(false), seperable(false) {}

		~tShader() {}

		void ProcessInterfaces()
		{
			//get all interfaces and resources

			//uniforms
			GLint numResources = 0;
			GLint resource = 0;

			std::vector<GLenum> supportedInterfaces = {};

			for (auto interfaceIter : interfaces)
			{

				glGetProgramInterfaceiv(pipelineHandle, interfaceIter, gl_active_resources, &numResources);

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
					glGetProgramInterfaceiv(pipelineHandle, interfaceIter, iter, &resource);

					GLint valueLength = 0;
					glGetProgramResourceiv(pipelineHandle, interfaceIter, iter, properties.size(), &properties[0], values.size(), &valueLength, &values[0]);

					nameData.resize(values[0]);
					glGetProgramResourceName(pipelineHandle, interfaceIter, iter, nameData.size(), NULL, &nameData[0]);
					std::string name((char*)&nameData[0], nameData.size() - 1);

					printf("index %d: %s %s @ location %d.\n", iter, typeToString(values[1]).c_str(), name.c_str(), values[3]);
				}
			}
		}

		/*
		* compile the shader from a given text file
		*/
		std::error_code Compile(std::string source)
		{
			//if the component hasn't been compiled yet
			if (!isCompiled)
			{
				char errorLog[512];
				GLint successful;

				if (!source.empty())
				{
					handle = glCreateShader(static_cast<unsigned int>(type));
					const char* str = source.c_str();
					glShaderSource(handle, 1, (const char**)&str, 0);
					glCompileShader(handle);

					glGetShaderiv(handle, gl_compile_status, &successful);
					glGetShaderInfoLog(handle, sizeof(errorLog), 0, errorLog);

					if (seperable)
					{
						pipelineHandle = glCreateProgram();
						glAttachShader(pipelineHandle, handle);

						glProgramParameteri(pipelineHandle, gl_program_separable, GL_TRUE);
						glLinkProgram(pipelineHandle);
					}

					if (successful != GL_TRUE)
					{
						printf("%s \n", errorLog);
						return error_t::shaderLoadFailed;
					}

					else
					{
						//ProcessInterfaces();
						isCompiled = GL_TRUE;
					}
				}
				else
				{
					return error_t::invalidSourceFile;
				}
			}
			else
			{
				//either the file name doesn't exist or the component has already been loaded
				return error_t::invalidFilePath;
			}
			return error_t::success;
		}

		/*
		* remove the shader from OpenGL
		*/
		void Shutdown()
		{
			glDeleteShader(handle);
			isCompiled = GL_FALSE;
		}

		/*
		* convert the given file to a single dimension c-string buffer
		*/
		std::error_code FileToBuffer(const std::string& path, std::string& bufferToFill)
		{
			FILE* file = fopen(path.c_str(), "rt");

			if (file == nullptr)
			{
				return error_t::invalidFilePath;
			}

			//get total byte in given file
			fseek(file, 0, SEEK_END);
			GLuint FileLength = ftell(file);
			fseek(file, 0, SEEK_SET);

			//allocate a file buffer and read the contents of the file
			std::string buffer(FileLength, '\0');
			fread(&buffer[0], sizeof(char), FileLength, file);

			fclose(file);
			bufferToFill = buffer;
			return error_t::success;
		}
	};

	/*
	* a TShaderProgram is a wrapper for an OpenGL shader program
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
		std::vector< tShader >			shaders;			/**< The components that the shader program is comprised of as a vector */
		GLuint							pipelineID;			/**< The GL pipeline ID for building modular shader programs */


		/*
		* basic constructor
		*/
		tShaderProgram() : 
		handle(0), isCompiled(false), pipelineID(0) {};

		/*
		* uses the given values to create an OpenGL shader program
		*/
		tShaderProgram(std::string programName,
			std::vector< std::string > programInputs,
			std::vector< std::string > programOutputs,
			std::vector< tShader > programShaders,
			bool saveBinary = false) :
			name(programName), inputs(programInputs),
			outputs(programOutputs), shaders(programShaders),
			pipelineID(0)
		{
			isCompiled = GL_FALSE;
			if (Compile(saveBinary) != error_t::success)
			{
				exit(0);
			};
			//get number of uniform blocks
			/*if (parseUniformBlockEvent != nullptr)
			{
				shaderBlocksEvent(handle);
			}*/
		};

		/*
		* another bare bones constructor
		*/
		tShaderProgram(std::string programName) : 
			name(programName), isCompiled(false), handle(0), pipelineID(0) {};

		tShaderProgram(std::string programName, GLuint programHandle) :
			name(programName), handle(programHandle), isCompiled(false), pipelineID(0) {}

		tShaderProgram(std::string programName, tShader computeShader, bool saveBinary = false)
			:name(programName), pipelineID(0)
		{
			shaders.push_back(computeShader);
			isCompiled = false;

			if (Compile(saveBinary) != error_t::success)
			{
				exit(0);
			}

		}

		~tShaderProgram() {}

		void ProcessInterfaces()
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
				glGetProgramInterfaceiv(handle, interfaceIter, gl_active_resources, &numResources); 
				const GLenum blockProperties[1] = { gl_num_active_variables };
				const GLenum activeUnifProp[1] = { gl_active_variables };
				const GLenum unifProperties[3] = { gl_name_length, gl_type, gl_location };

				for (int blockIter = 0; blockIter < numResources; blockIter++)
				{
					int numActiveUniforms = 0;
					glGetProgramResourceiv(handle, gl_uniform_block, blockIter, 1, blockProperties, 1, NULL, &numActiveUniforms);
					if (numActiveUniforms == 0)
					{
						return;
					}

					std::vector<int>	blockUniforms(numActiveUniforms);
					glGetProgramResourceiv(handle, gl_uniform_block, blockIter, 1, activeUnifProp, numActiveUniforms, NULL, &blockUniforms[0]);

					for (size_t uniformIter = 0; uniformIter < numActiveUniforms; uniformIter++)
					{
						int values[3];
						glGetProgramResourceiv(handle, gl_uniform, blockUniforms[uniformIter], 3, unifProperties, 3, NULL, values);

						std::vector<char> nameData(values[0]);
						glGetProgramResourceName(handle, gl_uniform, blockUniforms[uniformIter], nameData.size(), NULL, &nameData[0]);
						std::string name(nameData.begin(), nameData.end() - 1);
						printf("%s \n", name.c_str());
					}
				}
			}
		}

		/*
		* shut down the shader program. delete it from OpenGL
		*/
		void Shutdown()
		{
			glDeleteProgram(handle);

			for (auto& shader : shaders)
			{
				shader.Shutdown();
			}
			shaders.clear();
			inputs.clear();
			outputs.clear();
		}

		/*
		* compile the OpenGL shader program with the given information
		*/
		std::error_code Compile(bool saveBinary, bool pipelines = true)
		{
			handle = glCreateProgram();
			char errorLog[512];
			GLint successful = GL_FALSE;
			if (!isCompiled)
			{
				for (auto & shader : shaders)
				{
					//if (shader != nullptr)
					{
						glAttachShader(handle, shader.handle);
					}
				}

				// specify vertex input attributes
				for (size_t i = 0; i < inputs.size(); ++i)
				{
					glBindAttribLocation(handle, (GLuint)i, inputs[i].c_str());
				}

				// specify pixel shader outputs
				for (size_t i = 0; i < outputs.size(); ++i)
				{
					glBindFragDataLocation(handle, (GLuint)i, outputs[i].c_str());
				}

				if (saveBinary)
				{
					glProgramParameteri(handle, gl_program_binary_retrievable_hint, GL_TRUE);
				}

				glLinkProgram(handle);
				glGetProgramiv(handle, gl_link_status, &successful);

				if (successful != 1)
				{
					glGetProgramInfoLog(handle, sizeof(errorLog), 0, errorLog);
					printf("%s \n", errorLog);
					return error_t::shaderProgramLinkFailed;
				}

				isCompiled = GL_TRUE;
				ProcessInterfaces();

				/*if(successful)
				{
					glGenProgramPipelines(1, &pipelineID);
					unsigned int shaderBit = 0;
					for (auto iter : shaders)
					{
						if (iter->seperable)
						{
							switch (iter->type)
							{
							case shaderType_t::vertex:
							{
								glUseProgramStages(pipelineID, gl_vertex_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_vertex_shader_bit;
								break;
							}

							case shaderType_t::fragment:
							{
								glUseProgramStages(pipelineID, gl_fragment_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_fragment_shader_bit;
								break;
							}

							case shaderType_t::geometry:
							{
								glUseProgramStages(pipelineID, gl_geometry_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_geometry_shader_bit;
								break;
							}

							case shaderType_t::tessControl:
							{
								glUseProgramStages(pipelineID, gl_tess_control_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_tess_control_shader_bit;
								break;
							}

							case shaderType_t::tessEval:
							{
								glUseProgramStages(pipelineID, gl_tess_evaluation_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_tess_evaluation_shader_bit;
								break;
							}

							case shaderType_t::compute:
							{
								glUseProgramStages(pipelineID, gl_compute_shader_bit, iter->pipelineHandle);
								shaderBit |= gl_compute_shader_bit;
								break;
							}
							}
						}
					}

					//glUseProgramStages(pipelineID, shaderBit, handle);
					
					//glBindProgramPipeline(pipelineID);
					//glValidateProgramPipeline(pipelineID);

					GLint outParam = 0;
					//glGetProgramPipelineiv(pipelineID, gl_info_log_length, &outParam);

					GLsizei blarg = 512;
					char logBuffer[512];

					//glGetProgramInfoLog(pipelineID, outParam, &blarg, logBuffer);

					//printf("%s \n", logBuffer);

				}*/
				//if a shader successfully compiles then it will add itself to storage

				if (saveBinary)
				{
					GLint binarySize = 0;
					glGetProgramiv(handle, gl_program_binary_length, &binarySize);

					auto* buffer = (void*)malloc(binarySize);

					if (buffer == nullptr)
					{
						return error_t::shaderProgramCompileFailed;
					}

					GLenum binaryFormat = GL_NONE;

					glGetProgramBinary(handle, binarySize, NULL, &binaryFormat, buffer);

					std::string path;

					path += defaultBinaryPath;
					path += name;
					path += defaultProgramBinaryExtension;

					FILE* file = fopen(path.c_str(), "wb");
					fprintf(file, "%s\n", name.c_str());
					fprintf(file, "%i\n", binarySize);
					fprintf(file, "%i\n", binaryFormat);
					fwrite(buffer, binarySize, 1, file);
					fclose(file);
					path.clear();
				}


				return error_t::success;
			}
			return error_t::shaderProgramAlreadyExists;
		}
	};

	class tPipeline
	{
		//maximum of 5 linked 
		GLuint handle;

		//can contain stages for only a single stage in rendering pipeline
		//or for just a few stages

		//multiple program objects, each representing a section of the OpenGL pipeline,
		//can then be attached to a program pipeline object and matched together at 
		//runtine rather than link time

		//shaders 

		//hmm a vector of shaders with a specific flag?
		//

	};

	class tUniformBlock
	{
		std::string name;
		size_t ID;

		//what can we know about the block itself

	};

	class shaderManager
	{
		public:
			parseUniformBlockEvent_t parseUniformBlockEvent;

			std::vector< std::unique_ptr<tShaderProgram>>		shaderPrograms;    	/**< All loaded shader programs */
			std::vector< std::unique_ptr<tShader>>			shaders;			/**< All loaded shaders*/

			shaderManager(){}
			~shaderManager(){}

			/*
			* shuts down TinyShaders. deletes all OpenGL shaders and shader programs 
			* as well as calling shutdown on all shader and programs and clears all vectors.
			*/
			void Shutdown()
			{
				for (auto & shader : shaders)
				{
					shader->Shutdown();
					// delete shaders[iterator];
				}

				for (auto & shaderProgram : shaderPrograms)
				{
					shaderProgram->Shutdown();
					//delete shaderPrograms[iterator];
				}

				shaderPrograms.clear();
				//delete instance;
			}

			/*
			* returns a pointer to a TShaderProgram corresponding to the given name. returns nullptr if the TShaderProgram is not found
			*/
			std::error_code GetShaderProgramByName( std::string programName, tShaderProgram* outProgram )
			{
				if (!programName.empty())
				{
					for (auto & shaderProgram : shaderPrograms)
					{
						if (shaderProgram->name.compare(programName) == 0)
						{
							outProgram = shaderProgram.get();
							return error_t::success;
						}
					}
					return error_t::shaderProgramNotFound;
				}
				return error_t::invalidShaderProgramName;
			}

			/*
			* returns a pointer to a TShader corresponding to the given name. returns nullptr if the TShader is not found
			*/
			std::error_code GetShaderByName( std::string shaderName, tShader* outShader )
			{
				if (!shaderName.empty())
				{
					for (auto & shaderIter : shaders)
					{
						if (shaderIter->name.compare(shaderName) == 0)
						{
							outShader = shaderIter.get();
							return error_t::success;
						}
					}
					return error_t::shaderNotFound;
				}
				return error_t::invalidShaderName;
			}

			/*
			* load an OpenGL shader
			*/
			std::error_code LoadShader( std::string name, std::string shaderFile, shaderType_t shaderType, tShader* outShader = nullptr, bool seperable = true )
			{
				if (!name.empty())
				{
					std::unique_ptr<tShader> newShader(new tShader(name, shaderType, shaderFile));
					if (newShader->isCompiled)
					{
						shaders.push_back(std::move(newShader));
						outShader = shaders.back().get();
						return error_t::success;
					}
					return error_t::shaderCompileFailed;
				}
				return error_t::invalidString;
			}

			/*
			* loads all shaders and shader programs specified in a custom configuration file -- DEPRECATED
			*/
			/*std::error_code LoadShaderProgramsFromConfigFile(const std::string& configPath, bool saveBinary = false, std::vector<tShaderProgram*>* outPrograms = nullptr)
			{
					FILE* pConfigFile = fopen( configPath.c_str(), "r" );
					GLuint numInputs = 0;
					GLuint numOutputs = 0;
					GLuint numPrograms = 0;
					GLuint numShaders = 0;
					GLuint iterator = 0;

					std::vector< std::string > inputs, outputs, paths, names;
					std::vector< tShader* > localShaders;
					if ( pConfigFile )
					{
						//get the total number of shader programs
						fscanf( pConfigFile, "%u\n", &numPrograms );

						for ( GLuint programIter = 0;
							programIter <numPrograms;
							programIter++, paths.clear(), inputs.clear(), outputs.clear(), names.clear(), localShaders.clear() )
						{
							//get the name of the shader program
							auto* programName = new char[255];
							fscanf( pConfigFile, "%s\n", programName );
							printf( "%s\n", programName );

							//this is an anti-trolling measure. If a shader with the same name already exists then don't bother making a new one.
							if ( !ShaderProgramExists( programName ) )
							{
								//get the number of shader inputs
								fscanf( pConfigFile, "%u\n", &numInputs );

								//get all inputs
								for ( iterator = 0; iterator <numInputs; iterator++ )
								{
									auto* input = new char[255];
									fscanf( pConfigFile, "%s\n", input );
									inputs.emplace_back( input );
								}

								//get the number of shader outputs
								fscanf( pConfigFile, "%u\n", &numOutputs );

								//get all outputs
								for ( iterator = 0; iterator <numOutputs; iterator++ )
								{
									auto* output = new char[255];
									fscanf( pConfigFile, "%s\n", output );
									outputs.emplace_back( output );
								}

								//get number of shaders
								fscanf( pConfigFile, "%u\n", &numShaders );
								printf( "%u\n", numShaders );

								for( GLuint it = 0; it <numShaders; it++ )
								{
									auto* shaderName = new char[255];
									auto* shaderPath = new char[255];
									auto* shaderType = new char[255];

									//get shader name
									fscanf( pConfigFile, "%s\n", shaderName );
									printf( "%s\n", shaderName );

									//if the shader hasn't been loaded already then make a new one
									if( !ShaderExists( shaderName ) )
									{
										//remove printf calls?
										//get type
										fscanf( pConfigFile, "%s\n", shaderType );
										printf( "%s\n", shaderType );
										//get file path
										fscanf( pConfigFile, "%s\n", shaderPath );
										printf( "%s\n", shaderPath );

										shaderType_t newType;
										StringToShaderType((std::string)shaderType, newType);

										tShader* newShader = new tShader(shaderName, newType, shaderPath);
										if(newShader->isCompiled)
										{
											localShaders.push_back(std::move(newShader));
										}
										else
										{
											return error_t::shaderCompileFailed;
										}
									}

									else
									{
										//tell fscanf to skip a couple lines. very unsafe!
										fscanf( pConfigFile, "%*[^\n]\n %*[^\n]\n", nullptr );
										//if shader already exists then add an existing one from storage, it should already be compiled
										tShader* newShader = nullptr;
										GetShaderByName(shaderName, newShader);
										localShaders.push_back( newShader );
									}
								}

								std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram( programName, inputs, outputs, localShaders, saveBinary));

								if (newShaderProgram->isCompiled)
								{
									if (outPrograms != nullptr)
									{
										outPrograms->push_back(newShaderProgram.get());
									}
									shaderPrograms.push_back(std::move(newShaderProgram));
									continue;
								}
								fclose(pConfigFile);
								return error_t::shaderProgramCompileFailed;
							}
						}
						fclose( pConfigFile );
						return error_t::success;
					}
				return error_t::invalidFilePath;
			}

			std::error_code LoadProgramBinariesFromConfigFile( const std::string& configPath, std::vector<tShaderProgram*>* outPrograms = nullptr )
			{
				//open a file stream to binaries.txt
				GLuint numBinaries;
				FILE* configFile = fopen(configPath.c_str(), "r");
				if ( configFile )
				{
					fscanf(configFile, "%u", &numBinaries);
					fscanf(configFile, "%*[^\n]\n %*[^\n]\n", nullptr);
					for (unsigned int iter = 0; iter < numBinaries; iter++)
					{
						char binaryPath[255];
						fscanf(configFile, "%s \n", binaryPath);

						FILE* binaryFile = fopen(binaryPath, "rb");
						//std::ifstream file;
						char binaryName[255];
						GLuint binarySize = 0;
						GLuint binaryFormat = 0;

						fscanf(binaryFile, "%s \n", binaryName);
						fscanf(binaryFile, "%u \n", &binarySize);
						fscanf(binaryFile, "%u \n", &binaryFormat);

						auto* binaryBuffer = (void*)malloc(binarySize);
						fread(binaryBuffer, binarySize, 1, binaryFile);
						fclose(binaryFile);

						//load the buffer into OpenGL
						GLuint programHandle = glCreateProgram();
						glProgramBinary(programHandle, binaryFormat, binaryBuffer, binarySize);
						free(binaryBuffer);
						GLint isSuccessful = false;

						glGetProgramiv(programHandle, gl_link_status, &isSuccessful);

						if (isSuccessful)
						{
							//create a program object and load into the list
							std::unique_ptr<tShaderProgram> newProgram(new tShaderProgram(binaryName, programHandle));
							{
								if (newProgram->isCompiled)
								{
									shaderPrograms.push_back(std::move(newProgram));
									if (outPrograms != nullptr)
									{
										outPrograms->push_back(newProgram.get());
									}
								}
							}
						}
						else
						{
							fclose(configFile);
							return error_t::shaderProgramLinkFailed;
						}
					}
					fclose(configFile);
					return error_t::success;
				}
				return error_t::invalidFilePath;
			}

			std::error_code LoadShadersFromConfigFile( const std::string& configFile, std::vector<tShader*>* outShaders = nullptr)
			{
				FILE* pConfigFile = fopen( configFile.c_str(), "r+" );
				GLuint numShaders = 0;

				if( pConfigFile )
				{
					//get the number of shaders to load
					fscanf(pConfigFile, "%u\n", &numShaders);
					char* shaderName;
					char*	shaderType;
					char*	shaderPath;

					char empty[255] = "";

					for( GLuint iterator = 0; iterator <numShaders;
							iterator++, fscanf( pConfigFile, "\n\n" ) )
					{
						shaderName = empty;
						fscanf( pConfigFile, "%s\n", shaderName );

						if( !ShaderExists( shaderName ) )
						{
							shaderType = empty;
							fscanf( pConfigFile, "%s\n", shaderType );

							shaderPath = empty;
							fscanf( pConfigFile, "%s\n", shaderPath );

							shaderType_t typeVal;
							StringToShaderType(shaderType, typeVal);
							std::unique_ptr<tShader> newShader (new tShader(shaderName, typeVal, shaderPath ));
							if (newShader->isCompiled)
							{
								shaders.push_back(std::move(newShader));
								if (outShaders != nullptr)
								{
									outShaders->push_back(shaders.back().get());
								}
							}
							fclose(pConfigFile);
							return error_t::shaderCompileFailed;
						}
					}
					fclose(pConfigFile);
					return error_t::success;
				}
				return error_t::invalidFilePath;
			}

			std::error_code SaveShaderProgramsToConfigFile( const std::string& fileName )
			{
				//write total amount of shaders
				FILE* pConfigFile = fopen( fileName.c_str(), "w+" );

				if (pConfigFile)
				{

					fprintf(pConfigFile, "%i\n\n", (GLint)shaderPrograms.size());

					for (auto & shaderProgram : shaderPrograms)
					{
						//write program name
						fprintf(pConfigFile, "%s\n", shaderProgram->name.c_str());

						//write number of inputs
						fprintf(pConfigFile, "%i\n", (GLint)shaderProgram->inputs.size());

						//write inputs
						for (auto & input : shaderProgram->inputs)
						{
							fprintf(pConfigFile, "%s\n", input.c_str());
						}

						fprintf(pConfigFile, "%i\n", (GLint)shaderProgram->outputs.size());

						//write outputs
						for (auto & output : shaderProgram->outputs)
						{
							fprintf(pConfigFile, "%s\n", output.c_str());
						}

						//write number of shaders
						fprintf(pConfigFile, "%i\n", (GLint)shaderProgram->shaders.size());

						for (auto & shader : shaderProgram->shaders)
						{
							//write shader name
							fprintf(pConfigFile, "%s\n", shader->name.c_str());

							//write shader type
							fprintf(pConfigFile, "%s\n", ShaderTypeToString(shader->type).c_str());

							//write shader file path
							fprintf(pConfigFile, "%s\n", shader->filePath.c_str());
						}
					}
					fclose(pConfigFile);
					return error_t::success;
				}
				return error_t::invalidFilePath;
			}	*/

			/*
			* builds a new OpenGL shader program from already loaded shaders
			*/
			std::error_code BuildProgramFromShaders( const std::string& shaderName,
				const std::vector< std::string >& inputs,
				const std::vector< std::string >& outputs,
				const std::string& vertexShaderName,
				const std::string& fragmentShaderName,
				const std::string& geometryShaderName,
				const std::string& tessContShaderName,
				const std::string& tessEvalShaderName,
				tShaderProgram* outProgram = nullptr,
				bool saveBinary = false )
			{
					std::vector< tShader > shaderList;
					tShader vertexShader;// = nullptr;
					GetShaderByName(vertexShaderName, &vertexShader);
					tShader fragmentShader;// = nullptr;
					GetShaderByName(fragmentShaderName, &fragmentShader);
					tShader geometryShader;// = nullptr;
					GetShaderByName(geometryShaderName, &geometryShader);
					tShader tessControlShader;// = nullptr;
					GetShaderByName(tessContShaderName, &tessControlShader);
					tShader tessEvalShader;// = nullptr;
					GetShaderByName(tessEvalShaderName, &tessEvalShader);

					shaderList.push_back( vertexShader );
					shaderList.push_back( fragmentShader );
					shaderList.push_back( geometryShader );
					shaderList.push_back( tessControlShader );
					shaderList.push_back( tessEvalShader );

					std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram( shaderName, inputs, outputs, shaderList, saveBinary ));
					if (newShaderProgram->isCompiled)
					{
						shaderPrograms.push_back(std::move(newShaderProgram));
						if (outProgram != nullptr)
						{
							outProgram = shaderPrograms.back().get();
						}
						return error_t::success;
					}
					return error_t::shaderProgramLoadFailed;
			}

			std::error_code BuildProgramFromShaders(const std::string& shaderName,
				const std::string& computeShaderName,
				tShaderProgram* outProgram = nullptr,
				bool saveBinary = false)
			{
				std::vector< tShader > shaderList;
				tShader computeShader;
				GetShaderByName(computeShaderName, &computeShader);

				shaderList.push_back(computeShader);
				std::unique_ptr<tShaderProgram> newShaderProgram(new tShaderProgram(shaderName, shaderList[0], saveBinary));
				if (newShaderProgram->isCompiled)
				{
					shaderPrograms.push_back(std::move(newShaderProgram));
					if (outProgram != nullptr)
					{
						outProgram = shaderPrograms.back().get();
					}
					return error_t::success;
				}
				return error_t::shaderProgramLoadFailed;
			}

			/*
			* check if the shader program exists in TinyShaders.( has it been loaded and initialized? )
			*/
			GLboolean ShaderProgramExists( const std::string& shaderName )
			{
				if ( !shaderName.empty() )
				{
					if (!shaderPrograms.empty() )
					{
						for (auto & shaderProgram : shaderPrograms)
						{
							if ( shaderProgram != nullptr && shaderName.compare(shaderProgram->name) == 0)
							{
								return GL_TRUE;
							}
						}
						return GL_FALSE;
					}
					return GL_FALSE;
				}
				return GL_FALSE;
			}

			/*
			* check if the shader exists in TinyShaders. ( has it been loaded and initialized? )
			*/
			GLboolean ShaderExists( const std::string& shaderName )
			{
				if ( !shaderName.empty() )
				{
					if ( !shaders.empty() )
					{
						for (auto & shader : shaders)
						{
							if ( shader != nullptr && shaderName.compare(shader->name) )
							{
								return true;
							}
						}
						return false;
					}
					return false;
				}
				return false;
			}

			std::error_code LoadShaderFromBuffer( const std::string& name, const std::string& buffer, const shaderType_t& shaderType )
			{
				if ( !buffer.empty() )
				{
					if ( name.empty() )
					{
						if (!ShaderExists(name))
						{
							std::unique_ptr<tShader> newShader(new tShader(name, buffer, shaderType));
							if (newShader->isCompiled)
							{
								shaders.push_back(std::move(newShader));
								return error_t::success;
							}
							else
							{
								return error_t::shaderCompileFailed;
							}
						}
						return error_t::shaderNotFound;
					}
					return error_t::invalidShaderName;
				}
				return error_t::invalidString;
			}

		//private:


	};

	/*
* convert the given string to a shader type
*/
	std::error_code StringToShaderType( const std::string& typeString, shaderType_t& shaderTypeOut )
	{
		if( !typeString.empty() )
		{
			if ( typeString.compare("vertex") == 0 )
			{
				shaderTypeOut = shaderType_t::vertex;
				return error_t::success;
			}

			if ( typeString.compare("fragment") == 0 )
			{
				shaderTypeOut = shaderType_t::fragment;
				return error_t::success;
			}

			if ( typeString.compare("geometry") == 0 )
			{
				shaderTypeOut = shaderType_t::geometry;
				return error_t::success;
			}

			if ( typeString.compare("tessellation_Control") == 0 )
			{
				shaderTypeOut = shaderType_t::tessControl;
				return error_t::success;
			}

			if ( typeString.compare("tessellation_Evaluation") == 0 )
			{
				shaderTypeOut = shaderType_t::tessEval;
				return error_t::success;
			}

			if (typeString.compare("compute") == 0)
			{
				shaderTypeOut = shaderType_t::compute;
				return error_t::success;
			}

			return error_t::invalidShaderType;
		}
		return error_t::invalidString;
	}

	/*
	* convert the given shader type to a string
	*/
	std::string ShaderTypeToString( const shaderType_t& shaderType )
	{
		switch ( shaderType )
		{
		case shaderType_t::vertex:
			{
				return "vertex";
			}
		case shaderType_t::fragment:
			{
				return "fragment";
			}
		case shaderType_t::geometry:
			{
				return "geometry";
			}
		case shaderType_t::tessControl:
			{
				return "tessellation Control";
			}
		case shaderType_t::tessEval:
			{
				return "tessellation Evaluation";
			}
		case shaderType_t::compute:
			{
				return "compute";
			}
		}
		return nullptr;
	}
}
#endif
