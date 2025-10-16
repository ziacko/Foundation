#pragma once

struct abstractBlock_t
{
public:
	virtual ~abstractBlock_t() = default;

	// Common properties
	std::string name;
	int32_t bindingSlot;
	int32_t handle;

	// Pure virtual functions that must be implemented by derived classes
	//virtual void Initialize(GLuint inHandle, const GLenum& target = GL_UNIFORM_BUFFER) = 0;
	//virtual void Update(const GLenum& target = GL_UNIFORM_BUFFER, const GLenum& usage = GL_DYNAMIC_DRAW) = 0;
	//virtual void BindToSlot(const uint16_t& inSlot, const GLenum& target = GL_UNIFORM_BUFFER) = 0;

protected:
	abstractBlock_t() : bindingSlot(0), handle(0)
	{
	}

	abstractBlock_t(const std::string& blockName, const uint32_t slot)
		: name(blockName), bindingSlot(slot), handle(0)
	{

	}
};


//could split this between uniform and shader storage blocks
class uniform final : public abstractBlock_t
{
public:
	std::string type;

	int32_t location;
	int32_t araySize = -1;
	int32_t arrayStride = -1;
	int32_t metrixStride = -1;
	bool isRowMajor = false;
};

struct reflectionBlock_t final : public abstractBlock_t
{
	enum class type_e
	{
		uniform,
		storage,
		invalid,
	};

	uint32_t bindingHandle;
	uint32_t target;
	uint32_t usage;
	//alignment?
	uint16_t numActiveMembers;
	type_e blockType;

	// payloadTuple: pointer used for GPU uploads (raw bytes to glBufferData)
	std::pair<void*, uint64_t> payloadTuple;
	// hostPayloadTuple: owning pointer for complex host-side objects (e.g., structs with std::vector)
	std::pair<void*, uint64_t> hostPayloadTuple { nullptr, 0 };

	// Internal flag to avoid unintended per-frame SSBO re-uploads
	bool needsUpload = false;

	tsl::robin_map<std::string, uniform> members;

	//ok now we need functions to throw into this
	void Initialize(const GLenum& inUsage = GL_DYNAMIC_DRAW)
	{
		// Assign binding index and create buffer object
		glGenBuffers(1, &bindingHandle);
		usage = inUsage;
		target = (blockType == type_e::uniform) ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
		
		// Ensure first upload happens once after initialization
		needsUpload = true;
		// Initialize data (if any) and bind buffer to binding point
		Update();
		glBindBufferBase(target, bindingSlot, bindingHandle);
	}

	template <typename T>
	T* GetPayload()
	{
		// Prefer returning the host-side object when present
		if (hostPayloadTuple.first)
		{
			return static_cast<T*>(hostPayloadTuple.first);
		}
		return static_cast<T*>(payloadTuple.first);
	}

	template <typename T>
	void SetPayload(T payload)
	{
		// Store an owning copy for the host
		hostPayloadTuple.first = static_cast<void*>(new T(payload));
		hostPayloadTuple.second = sizeof(T);
		// For uniforms, we upload the raw struct bytes
		payloadTuple = hostPayloadTuple;
		// Mark for upload on next Update (avoids overwriting SSBO each frame unless data changed)
		needsUpload = true;
	}

	template <typename T>
	void SetPayloadCustom(T payload, uint32_t size)
	{
		// Store an owning copy for the host (e.g., a struct that contains a std::vector)
		auto* hostObj = new T(payload);
		hostPayloadTuple.first = static_cast<void*>(hostObj);
		hostPayloadTuple.second = sizeof(T);

		// For SSBOs we want to upload the contiguous element data, not the struct header.
		// This implementation assumes T has a member named 'cells' that is a contiguous container (std::vector-like).
		if constexpr (requires { hostObj->cells.data(); hostObj->cells.size(); })
		{
			payloadTuple.first = static_cast<void*>(hostObj->cells.data());
			payloadTuple.second = size; // caller provides explicit byte size of the contiguous data
		}
		else
		{
			// Fallback: upload the object bytes directly
			payloadTuple.first = hostPayloadTuple.first;
			payloadTuple.second = size;
		}
		// Mark for upload on next Update
		needsUpload = true;
	}

	void Update(std::pair<void*, uint64_t>* inData = nullptr)
	{
		glBindBuffer(target, bindingHandle);
		if (inData != nullptr)
		{
			payloadTuple = *inData;
			// Bind the actual buffer object handle, not the binding slot
			
			auto size = payloadTuple.second;
			if (size > 0)
			{
				// caller provides explicit size and data
				glBufferData(target, size,  payloadTuple.first, usage);
				needsUpload = false;
			}
			return;
		}
		
		// default path uses the embedded data field
		auto size = payloadTuple.second;
		if (target == GL_UNIFORM_BUFFER)
		{
			if (size > 0 && size < 16)
			{
				// In std140, uniform blocks are rounded up to a multiple of 16 bytes.
				// Allocate a 16-byte buffer, then upload the actual data with SubData.
				glBufferData(target, 16, nullptr, usage);
				glBufferSubData(target, 0, size, payloadTuple.first);
			}
			else if (size > 0)
			{
				glBufferData(target, size, payloadTuple.first, usage);
			}
			else
			{
				// No data provided; allocate zero-sized buffer (legal in GL) to ensure buffer is created
				glBufferData(target, 0, nullptr, usage);
			}
			return;
		}


		if (target == GL_SHADER_STORAGE_BUFFER)
		{
			if (!needsUpload)
			{
				// Keep buffer bound but do not overwrite GPU-side contents
				return;
			}
			if (size > 0 && size < 32)
			{
				glBufferData(target, 32, nullptr, usage);
				glBufferSubData(target, 0, size, payloadTuple.first);
			}
			else if (size > 0)
			{
				glBufferData(target, size, payloadTuple.first, usage);
			}
			else
			{
				glBufferData(target, 0, nullptr, usage);
			}
			needsUpload = false;
		}
	}

	void Override(const uint16_t inUniformHandle, const GLenum& inUsage = GL_DYNAMIC_DRAW, const size_t& dataSize = 0, const void* inData = nullptr)
	{
		//ok so this is for overriding the data in existing shader storage buffers
		//might have to look for a better system later
		glBindBufferBase(target, inUniformHandle, bindingHandle);
		usage = inUsage;
		if (dataSize > 0 && inData != nullptr)
		{
			glBufferData(target, dataSize, inData, inUsage);
			glFinish();
		}
	}

	void BindToSlot(const uint16_t& inUniformHandle)
	{
		// Bind this buffer object to the specified binding slot
		glBindBufferBase(target, inUniformHandle, bindingHandle);
		// Remember the binding slot, not overwrite the buffer handle
		this->bindingSlot = inUniformHandle;
	}

	// Write a sub-range of data into this buffer (useful for SSBO updates)
	void Write(const void* data, const uint64_t& size, const uint64_t& offset = 0) const
	{
		if (data == nullptr || size == 0) return;
		glBindBuffer(target, bindingHandle);
		glBufferSubData(target, offset, size, data);
	}

	// Read a sub-range of data back from this buffer into 'out'
	void Read(void* out, const uint64_t& size, const uint64_t& offset = 0) const
	{
		if (out == nullptr || size == 0) return;
		glBindBuffer(target, bindingHandle);
		glGetBufferSubData(target, offset, size, out);
	}
};

//ok let's make a handler class for these buffers
struct bufferHandler_t
{
	//uniform blocks
	//shader storage blocks
	//general uniforms

	tsl::robin_map<std::string, reflectionBlock_t> uniformBlocks;
	tsl::robin_map<std::string, reflectionBlock_t> shaderStorageBlocks;
	tsl::robin_map<std::string, uniform> uniforms;
};
