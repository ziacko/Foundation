 
#pragma once

namespace command
{
	struct base;

	typedef void (APIENTRY* PFN_EXECUTE) (const base* __restrict pParams);

	struct base
	{
		PFN_EXECUTE pfnExecute;
	};

	struct DrawElements_c : public base
	{
		GLenum mode;
		GLsizei count;
		GLenum type;
		GLvoid* indices;

		static void APIENTRY execute(const DrawElements_c* __restrict pParams)
		{
			glDrawElements(pParams->mode, pParams->count, pParams->type, pParams->indices);
		}
	};

	struct DrawArrays_c : public base
	{
		GLenum mode;
		GLint first;
		GLsizei count;

		static void APIENTRY execute(const DrawArrays_c* __restrict pParams)
		{
			glDrawArrays(pParams->mode, pParams->first, pParams->count);
		}
	};

	struct UseProgram_c : public base
	{
		GLuint program;

		static void APIENTRY execute(const UseProgram_c* __restrict pParams)
		{
			glUseProgram(pParams->program);
		}
	};

	struct PolygonMode_c : public base
	{
		GLenum face;
		GLenum mode;

		static void APIENTRY execute(const PolygonMode_c* __restrict pParams)
		{
			glPolygonMode(pParams->face, pParams->mode);
		}
	};

	struct clear_c : public base
	{
		GLbitfield mask;

		static void APIENTRY execute(const clear_c* __restrict pParams)
		{
			glClear(pParams->mask);
		}
	};

	struct bindVertexArray_c : public base
	{
		GLuint arrayHandle;

		static void APIENTRY execute(const bindVertexArray_c* __restrict pParams)
		{
			glBindVertexArray(pParams->arrayHandle);
		}
	};

	union ALL_PACKETS
	{
	public:
		PFN_EXECUTE			execute;
	private:
		base 				Base;
		DrawArrays_c		drawArrays;
		UseProgram_c		useProgram;
		PolygonMode_c		polygonMode;
		clear_c				clear;
		bindVertexArray_c	bindVertexArray;
	};

	enum functionIndex
	{
		DrawArrays = 0,
		useProgram,
		last = useProgram
	};

	class bufferStream
	{
	public:

		unsigned int 	maxPackets;
		unsigned int 	numPackets;
		ALL_PACKETS* 	packets;

		bufferStream(int maxPackets = 5)
		{
			this->maxPackets = maxPackets;
			numPackets = 0;
			packets = nullptr;
		}

		template <typename T>
		T* NextPacket()
		{
			return reinterpret_cast<T*>(&packets[numPackets++]);
		}

		void Init(unsigned int maxPackets)
		{
			this->maxPackets = maxPackets;
			numPackets = 0;
			packets = new ALL_PACKETS[maxPackets];
			memset(packets, 0, maxPackets * sizeof(ALL_PACKETS));
		}

		void UseProgram(GLuint program)
		{
			UseProgram_c* __restrict pPacket = NextPacket<UseProgram_c>();

			pPacket->pfnExecute = PFN_EXECUTE(UseProgram_c::execute);
			pPacket->program = program;
		}

		void DrawArrays(GLenum mode, GLint first, GLsizei count)
		{
			DrawArrays_c* __restrict pPacket = NextPacket<DrawArrays_c>();

			pPacket->pfnExecute = PFN_EXECUTE(DrawArrays_c::execute);

			pPacket->mode = mode;
			pPacket->first = first;
			pPacket->count = count;
		}

		void PolygonMode(GLenum face, GLenum mode)
		{
			PolygonMode_c* __restrict pPacket = NextPacket<PolygonMode_c>();
			pPacket->pfnExecute = PFN_EXECUTE(PolygonMode_c::execute);

			pPacket->face = face;
			pPacket->mode = mode;
		}

		void Clear(GLbitfield mask)
		{
			clear_c* __restrict pPacket = NextPacket<clear_c>();
			pPacket->pfnExecute = PFN_EXECUTE(clear_c::execute);

			pPacket->mask = mask;
		}

		void BindVertexArray(GLuint arrayHandle)
		{
			bindVertexArray_c* __restrict pPacket = NextPacket<bindVertexArray_c>();
			pPacket->pfnExecute = PFN_EXECUTE(bindVertexArray_c::execute);

			pPacket->arrayHandle = arrayHandle;
		}

		void Execute()
		{
			const ALL_PACKETS* __restrict pPacket;

			if (numPackets == 0)
			{
				return;
			}
			int test = 0; 
			for (pPacket = packets; pPacket->execute != nullptr && test < numPackets; pPacket++, test++)
			{
				
				pPacket->execute((base*)pPacket);
			}

			numPackets = 0;
		}
	};

}


