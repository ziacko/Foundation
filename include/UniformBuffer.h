#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

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
		bufferHandle = NULL;
		uniformHandle = NULL;
		data = CreateBaseBuffer();
		//BuildBuffer();
	}

	void Update(void* paramData, GLuint paramBufferHandle, GLintptr offset, GLuint bufferSize, GLenum target, GLenum usage)
	{
		glBindBuffer(target, paramBufferHandle);
		glBufferSubData(target, offset, bufferSize, paramData);
	}

	void Setup(void* inData, GLuint& outBufferHandle, GLintptr offset, GLuint bufferSize, GLuint inUniformHandle, GLenum target, GLenum usage)
	{
		this->data = inData;
		glGenBuffers(1, &outBufferHandle);
		this->bufferHandle = outBufferHandle;
		this->uniformHandle = inUniformHandle;
		Update(this->data, this->bufferHandle, offset, bufferSize, target, usage);
		glBindBufferBase(target, this->uniformHandle, this->bufferHandle);
	}

	void* CreateBaseBuffer()
	{
		return (void*)malloc(sizeof(*this) - (sizeof(GLuint) * 2));
	}

	virtual void* GetBuffer() = 0;

	virtual void BuildBuffer() = 0;

	template<typename t>
	void AppendBuffer(t object, void*& buffer)
	{
		memcpy(buffer, &object, sizeof(object));
		buffer = (void*)(((char*)buffer) + sizeof(object));
		dataSize += sizeof(object);
	}
};

#endif