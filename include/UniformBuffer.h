#pragma once

class uniformBuffer_t
{
public:

	GLuint bufferHandle;
	GLuint uniformHandle;

	GLuint dataSize;
	void* data;

	uniformBuffer_t()
	{
		dataSize = 0;
		bufferHandle = 0;
		uniformHandle = 0;
		data = CreateBaseBuffer();
		//BuildBuffer();
	}

	virtual ~uniformBuffer_t() { Shutdown(); }

	static void Update(const void* paramData, const GLuint paramBufferHandle, const GLintptr offset, const GLuint bufferSize, const GLenum target, const GLenum usage)
	{
		glBindBuffer(target, paramBufferHandle);
		glBufferSubData(target, offset, bufferSize, paramData);
	}

	void Setup(void* inData, GLuint& outBufferHandle, const GLintptr offset, const GLuint bufferSize, const GLuint inUniformHandle, const GLenum target, const GLenum usage)
	{
		this->data = inData;
		glGenBuffers(1, &outBufferHandle);
		this->bufferHandle = outBufferHandle;
		this->uniformHandle = inUniformHandle;
		Update(this->data, this->bufferHandle, offset, bufferSize, target, usage);
		glBindBufferBase(target, this->uniformHandle, this->bufferHandle);
	}

	void* CreateBaseBuffer() const
	{
		return (void*)malloc(sizeof(*this) - (sizeof(GLuint) * 2));
	}

	template<typename t>
	void AppendBuffer(t object, void*& buffer)
	{
		memcpy(buffer, &object, sizeof(object));
		buffer = static_cast<void*>(static_cast<char*>(buffer) + sizeof(object)); //i hate this eyesore
		dataSize += sizeof(object);
	}

	void Shutdown() const
	{
		glDeleteBuffers(1, &bufferHandle);
	}
};