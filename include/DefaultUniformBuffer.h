#pragma once

//change this into a payload system using templates
template<typename bufferType>
class bufferHandler_t
{
public:

	bufferHandler_t()
	{
		data = bufferType();
		bufferHandle = 0;
		uniformHandle = 0;
	}

	bufferHandler_t(bufferType payload)
	{
		this->data = payload;
		bufferHandle = 0;
		uniformHandle = 0;
	}

	//ok now we need functions to throw into this
	void Initialize(GLuint uniformHandle, const GLenum& target = GL_UNIFORM_BUFFER, const GLenum& usage = GL_DYNAMIC_DRAW)
	{
		this->uniformHandle = uniformHandle;
		glGenBuffers(1, &bufferHandle);
		Update(target, usage);
		glBindBufferBase(target, uniformHandle, bufferHandle);
	}

	void SetupUniforms(const GLuint& programHandle, const std::string& name, const GLuint& blockBindingIndex)
	{
		uniformHandle = glGetUniformBlockIndex(programHandle, name.c_str());
		glUniformBlockBinding(programHandle, uniformHandle, blockBindingIndex);
	}

	void Update(const GLenum& target = GL_UNIFORM_BUFFER, const GLenum& usage = GL_DYNAMIC_DRAW, const size_t& dataSize = 0, const void* inData = nullptr)
	{
		glBindBuffer(target, bufferHandle);
		if(dataSize > 0 && inData != nullptr)
		{
			glBufferData(target, dataSize, inData, usage);
		}
		else
		{
			glBufferData(target, sizeof(data), &data, usage);
		}
		
		//printf("%i \n", sizeof(data));
	}

	void Override(const uint16_t uniformHandle, const GLenum& target = GL_UNIFORM_BUFFER, const GLenum& usage = GL_DYNAMIC_DRAW, const size_t& dataSize = 0, const void* inData = nullptr) const
	{
		//ok so this is for overriding the data in existing shader storage buffers
		//might have to look for a better system later
		//glBindBuffer(target, bufferHandle); //i don't think re-setting the bufferhandle is needed here
		glBindBufferBase(target, uniformHandle, bufferHandle);
		if (dataSize > 0 && inData != nullptr)
		{
			glBufferData(target, dataSize, inData, usage);
			glFinish();
		}
	}

	void BindToSlot(const uint16_t& uniformHandle, const GLenum& target = GL_UNIFORM_BUFFER)
	{
		//glBindBuffer(target, bufferHandle);
		glBindBufferBase(target, uniformHandle, bufferHandle);
		//Update(target, gl_dynamic_draw);
		this->uniformHandle = uniformHandle;
	}

	bufferType data;
	uint32_t bufferHandle;
	uint32_t uniformHandle;
};

class defaultUniformBuffer// : public uniformBuffer_t
{
public:

	glm::mat4			projection;
	glm::mat4			view;
	glm::mat4			translation;
	glm::vec2			resolution;
	glm::vec2			mousePosition;
	GLfloat				deltaTime;
	GLfloat				totalTime;
	GLfloat				framesPerSec;
	GLuint				totalFrames;

	defaultUniformBuffer(const glm::mat4& projection, const glm::mat4& view,
			const glm::mat4& translation = glm::mat4( 1 ), const glm::ivec2 resolution = defaultWindowSize ):
		mousePosition(),
		deltaTime(0),
		totalTime(0),
		framesPerSec(0)
	//: uniformBuffer_t()
	{
		//BuildBuffer();
		//uniformBuffer_t();
		this->projection = projection;
		this->view = view;
		this->translation = translation;
		this->resolution = resolution;
		totalFrames = 1;
	}

	defaultUniformBuffer(const camera_t& defaultCamera)// : uniformBuffer_t()
		: mousePosition(), deltaTime(0), totalTime(0), framesPerSec(0)
	{
		//uniformBuffer_t();
		//BuildBuffer();
		this->projection = defaultCamera.projection;
		this->view = defaultCamera.view;
		this->translation = defaultCamera.translation;
		this->resolution = defaultCamera.resolution;
		totalFrames = 1;
	}

	defaultUniformBuffer(): projection(), view(), translation(), resolution(), mousePosition(), deltaTime(0),
	                        totalTime(0),
	                        framesPerSec(0),
	                        totalFrames(0)
	{
	}
};
