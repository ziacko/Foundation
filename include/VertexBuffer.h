#pragma once

class vertexBuffer_t
{
public:

	GLuint		bufferHandle;
	GLuint		indexBufferHandle;
	GLuint		vertexArrayHandle;

	vertexBuffer_t()
	{
		bufferHandle = 0;
		vertexArrayHandle = 0;
		indexBufferHandle = 0;
	}

	void SetupDefault()
	{
		GLfloat quadVerts[] =
		{
			//Triangle 1
			-1.0f,  1.0f,  0.0f, 1.0f,  // top-left
			1.0f,  1.0f,  0.0f, 1.0f, // top-right
			1.0f, -1.0f,  0.0f, 1.0f, // bottom-left

			// Triangle 2
			-1.0f, -1.0f,  0.0f, 1.0f, // bottom-left
			-1.0f,  1.0f,  0.0f, 1.0f, // top-left
			1.0f, -1.0f,  0.0f, 1.0f // bottom-right
		};

		const std::vector<unsigned int> indices = { 0, 1, 2, 3, 4, 5 };

		glGenVertexArrays(1, &vertexArrayHandle);
		glBindVertexArray(vertexArrayHandle);

		//load vertex buffer
		glGenBuffers(1, &bufferHandle);
		glBindBuffer(gl_array_buffer, bufferHandle);
		glBufferData(gl_array_buffer, sizeof(float) * 4 * 6, quadVerts, gl_static_draw);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (char*)(sizeof(float) * 4));

		//load index buffer
		glGenBuffers(1, &indexBufferHandle);
		glBindBuffer(gl_element_array_buffer, indexBufferHandle);
		glBufferData(gl_element_array_buffer, sizeof(unsigned int) * 6, indices.data(), gl_static_draw);

		glBindBuffer(gl_array_buffer, 0);
		glBindBuffer(gl_element_array_buffer, 0);
	}

	void SetupCustom(const glm::vec2& extents)
	{
		GLfloat quadVerts[] =
		{
			0.0f, 0.0f, 1.0f, 1.0f,
			extents.x, 0.0f, 1.0f, 1.0f,
			0.0f, extents.y, 1.0f, 1.0f,

			0.0f, extents.y, 1.0f, 1.0f,
			extents.x, 0.0f, 1.0f, 1.0f,
			extents.x, extents.y, 1.0f, 1.0f,
		};

		std::vector<unsigned int> indices = { 0, 1, 2, 3, 4, 5 };

		glGenVertexArrays(1, &vertexArrayHandle);
		glBindVertexArray(vertexArrayHandle);

		//load vertex buffer
		glGenBuffers(1, &bufferHandle);
		glBindBuffer(gl_array_buffer, bufferHandle);
		glBufferData(gl_array_buffer, sizeof(float) * 4 * 6, quadVerts, gl_static_draw);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (char*)(sizeof(float) * 4));

		//load index buffer
		glGenBuffers(1, &indexBufferHandle);
		glBindBuffer(gl_element_array_buffer, indexBufferHandle);
		glBufferData(gl_element_array_buffer, sizeof(unsigned int) * 6, indices.data(), gl_static_draw);

		glBindBuffer(gl_array_buffer, 0);
		glBindBuffer(gl_element_array_buffer, 0);
	}

	void UpdateBuffer(const glm::vec2& extents) const
	{
		GLfloat quadVerts[] =
		{
			0.0f, 0.0f, 1.0f, 1.0f,
			extents.x, 0.0f, 1.0f, 1.0f,
			0.0f, extents.y, 1.0f, 1.0f,

			0.0f, extents.y, 1.0f, 1.0f,
			extents.x, 0.0f, 1.0f, 1.0f,
			extents.x, extents.y, 1.0f, 1.0f,
		};

		glBindBuffer(gl_array_buffer, bufferHandle);
		glBufferData(gl_array_buffer, sizeof(float) * 24, quadVerts, gl_static_draw);
	}
};