#pragma once

#include <scene.h>
#include <cstdlib>
#include "UniformBuffer.h"

class golScene : public scene
{
public:

	enum cellState_t
	{
		EMPTY = 0,
		ALIVE,
		DEAD
	};

	class golSettings_t// : public uniformBuffer_t
	{
	public:
		
		glm::vec4		aliveColor;
		glm::vec4		deadColor;
		glm::vec4		emptyColor;
		float			dimensions; // change to vec2 for more flexibility

		explicit golSettings_t(
			float dimensions = 100, glm::vec4 aliveColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), //green
			glm::vec4 deadColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), //red
			glm::vec4 emptyColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)) //blue
		{
			this->dimensions = dimensions;

			this->aliveColor = aliveColor;
			this->deadColor = deadColor;
			this->emptyColor = emptyColor;
		}

		~golSettings_t() = default;
	};

	struct cells_t
	{
		std::vector<cellState_t>		cells;

		cells_t()
		{

		}

		cells_t(golScene* gol, float dimensions)
		{
			std::srand(gol->randomSeed);
			for (size_t cellIter = 0; cellIter < (size_t)dimensions * dimensions; cellIter++)
			{
				size_t newRand = std::rand() % 100;
				cells.push_back((cellState_t)(newRand < gol->cellProbability));
			}
		}

		~cells_t() = default;
	};

	explicit golScene(GLdouble tickDelay = 0.1f, GLuint randomSeed = 666,
		GLuint cellProbability = 90, const char* windowName = "Ziyad Barakat's portfolio (game of life)",
		camera_t golCamera = camera_t(), const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, golCamera, shaderConfigPath)
	{
		this->tickDelay = tickDelay;
		this->randomSeed = randomSeed;
		this->cellProbability = cellProbability;

		//cellBuffer = new cells_t(this, dimensions);
		cellDimensions = glm::vec2(0);
		currentTickDelay = 0.0f;
	}

	~golScene(void) override {};

protected:

	golSettings_t*		gol;
	cells_t*			cellBuffer;
	GLdouble							tickDelay;
	GLdouble							currentTickDelay;
	GLuint								randomSeed;
	GLuint								cellProbability;
	glm::vec2							cellDimensions;


	void Update() override
	{
		scene::Update();
		//cellBuffer->Update(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, sizeof(int) * cellBuffer->cells.size(), cellBuffer->cells.data());
		//UpdateBuffer(gol, gol->bufferHandle, sizeof(*gol), gl_uniform_buffer, gl_dynamic_draw);
		//UpdateBuffer(cellBuffer->cells.data(), cellBuffer->bufferHandle, sizeof(int) * cellBuffer->cells.size(), gl_shader_storage_buffer, gl_dynamic_draw);
		//UpdateDefaultBuffer();

		if (currentTickDelay < tickDelay)
		{
			currentTickDelay += clock.GetDeltaTime();
		}

		else
		{
			TickOver(); //ok tickover might be the problem
			currentTickDelay = 0;
		}
	}

	void Draw() override
	{
		defProgram.Use();
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, gol->dimensions * gol->dimensions);
	}

	void CheckNode(cellState_t CurrentState, unsigned int& neighborCount, unsigned int& deadNeighborCount)
	{
		switch (CurrentState)
		{
		case cellState_t::EMPTY:
		{
			break;
		}
			
		case cellState_t::ALIVE:
		{
			neighborCount++;
			break;

		}
			
		case cellState_t::DEAD:
		{
			deadNeighborCount++;
			break;
		}
			
		default:
			break;
		}
	}

	void TickOver()
	{
		for (unsigned int cellIndex = 0; cellIndex < gol->dimensions * gol->dimensions; cellIndex++)
		{
			unsigned int column = cellIndex % (unsigned int)gol->dimensions;
			unsigned int row = cellIndex / (unsigned int)gol->dimensions;
			if (cellBuffer->cells[cellIndex] == EMPTY)
			{
				continue;
			}

			unsigned int neighborCount = 0;
			unsigned int deadNeighborCount = 0;
			//for convenience
			unsigned int dimensionMinOne = (unsigned int)gol->dimensions - 1;

			if (cellIndex < gol->dimensions)
			{
				//check if the Item is not in the last column
				if (cellIndex % (unsigned int)gol->dimensions == 0)
				{
					//next row, next column
					CheckNode(cellBuffer->cells[cellIndex + dimensionMinOne + 1], neighborCount, deadNeighborCount);
					//next row, this column
					CheckNode(cellBuffer->cells[cellIndex + dimensionMinOne], neighborCount, deadNeighborCount);
					//this row, next column
					CheckNode(cellBuffer->cells[cellIndex + 1], neighborCount, deadNeighborCount);
				}

				//check if its the last column in the row
				else if (cellIndex % dimensionMinOne == 0)
				{
					//next row, last column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne) - 1], neighborCount, deadNeighborCount);
					//next row, this column
					CheckNode(cellBuffer->cells[cellIndex + dimensionMinOne], neighborCount, deadNeighborCount);
					//this row, last column
					CheckNode(cellBuffer->cells[cellIndex - 1], neighborCount, deadNeighborCount);
				}

				else
				{
					//5 neighbors to consider
					//this row, last column
					CheckNode(cellBuffer->cells[cellIndex - 1], neighborCount, deadNeighborCount);
					//this row, next column
					CheckNode(cellBuffer->cells[cellIndex + 1], neighborCount, deadNeighborCount);
					//next row, last column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne) - 1], neighborCount, deadNeighborCount);
					//next row, this column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne)], neighborCount, deadNeighborCount);
					//next row, next column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne) + 1], neighborCount, deadNeighborCount);
				}
			}

			//check if node is in the last row
			else if (cellIndex >
				(((gol->dimensions * gol->dimensions) - gol->dimensions) - 1)
				&& cellIndex < ((gol->dimensions * gol->dimensions) - 1))
			{
				//check if the column is a multiple of the dimension (the first column)
				if ((cellIndex % (unsigned int)gol->dimensions) == 0)
				{
					//last row, next column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) + 1], neighborCount, deadNeighborCount); //lower middle node
																									//last row, this column
					CheckNode(cellBuffer->cells[cellIndex - dimensionMinOne], neighborCount, deadNeighborCount); //lower middle node
																							  //this row, next column
					CheckNode(cellBuffer->cells[cellIndex + 1], neighborCount, deadNeighborCount); //lower middle node
				}

				else if ((cellIndex % dimensionMinOne == 0))
				{
					//last row, this column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne)], neighborCount, deadNeighborCount); //lower middle node
																								//last row, last column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) - 1], neighborCount, deadNeighborCount); //lower middle node
																									//this row, last column
					CheckNode(cellBuffer->cells[cellIndex - 1], neighborCount, deadNeighborCount); //lower middle node
				}

				else
				{
					//5 neighbors to consider
					//this row, last column
					CheckNode(cellBuffer->cells[cellIndex - 1], neighborCount, deadNeighborCount);
					//this row, next column
					CheckNode(cellBuffer->cells[cellIndex + 1], neighborCount, deadNeighborCount);
					//last row, last column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) - 1], neighborCount, deadNeighborCount);
					//last row, this column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne)], neighborCount, deadNeighborCount);
					//last row, next column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) + 1], neighborCount, deadNeighborCount);
				}
			}

			else
			{
				if (cellIndex < (gol->dimensions * gol->dimensions) - 1)
				{
					//8 neighbors to check
					//last row, last column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) - 1], neighborCount, deadNeighborCount);
					//last row, this column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne)], neighborCount, deadNeighborCount);
					//last row, next column
					CheckNode(cellBuffer->cells[(cellIndex - dimensionMinOne) + 1], neighborCount, deadNeighborCount);
					//this row, last column
					CheckNode(cellBuffer->cells[cellIndex - 1], neighborCount, deadNeighborCount);
					//this row, next column
					CheckNode(cellBuffer->cells[cellIndex + 1], neighborCount, deadNeighborCount);
					//next row, last column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne) - 1], neighborCount, deadNeighborCount);
					//next row, this column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne)], neighborCount, deadNeighborCount);
					//next row, next column
					CheckNode(cellBuffer->cells[(cellIndex + dimensionMinOne) + 1], neighborCount, deadNeighborCount);
				}
			}

			if (neighborCount < 2 && cellBuffer->cells[cellIndex] == ALIVE)
			{
				cellBuffer->cells[cellIndex] = DEAD;
			}

			else if (neighborCount >= 2 && cellBuffer->cells[cellIndex] == ALIVE)
			{
				if (neighborCount > 3)
				{
					cellBuffer->cells[cellIndex] = DEAD;
				}

				else
				{
					cellBuffer->cells[cellIndex] = ALIVE;
				}
			}

			else if (neighborCount == 3 && cellBuffer->cells[cellIndex] == DEAD)
			{
				cellBuffer->cells[cellIndex] = ALIVE;
			}
		}
	}

	void InitializeUniforms() override 
	{
		scene::InitializeUniforms();
		auto golBlock = &bufferHandler.uniformBlocks["GOLSettings"];
		golBlock->SetPayload<golSettings_t>(golSettings_t());
		gol = golBlock->GetPayload<golSettings_t>();

		auto cellBlock = &bufferHandler.shaderStorageBlocks["GOLStatus"];
		auto cells = cells_t(this, gol->dimensions);
		cellBlock->SetPayloadCustom<cells_t>(cells, GLuint(sizeof(int) * cells.cells.size()));
		cellBuffer = cellBlock->GetPayload<cells_t>();

		cellDimensions = glm::vec2(defaultPayload->resolution.x / gol->dimensions, defaultPayload->resolution.y / gol->dimensions);
		defaultVertexBuffer.SetupCustom((cellDimensions));
	}

	virtual void Resize(const tWindow* window, glm::ivec2 dimensions = glm::ivec2(0)) override
	{
		glViewport(0, 0, window->GetSettings().resolution.x, window->GetSettings().resolution.y);
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
		defaultPayload->projection = glm::ortho(0.0f, (GLfloat)window->GetSettings().resolution.x, (GLfloat)window->GetSettings().resolution.y, 0.0f, 0.01f, 10.0f);
		cellDimensions = glm::ivec2(defaultPayload->resolution.x / gol->dimensions, defaultPayload->resolution.y / gol->dimensions);

		//UpdateBuffer(defaultUniform, defaultUniform->bufferHandle, sizeof(*defaultUniform), gl_uniform_buffer, gl_dynamic_draw);
		if (dimensions == glm::ivec2(0))
		{
			defaultVertexBuffer.UpdateBuffer(defaultPayload->resolution);
		}

		else
		{
			defaultVertexBuffer.UpdateBuffer(cellDimensions);
		}
	}

	virtual void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t>& dimensions) override
	{
		Resize(window, cellDimensions);
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		Resize(window, cellDimensions);
	}
};
