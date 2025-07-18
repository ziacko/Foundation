#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

struct FBODescriptor : public textureDescriptor
{
	enum class attachmentType_e
	{
		color,
		depth,
		stencil,
		depthAndStencil
	};

	FBODescriptor(GLuint sampleCount = 1, attachmentType_e attachmentType = attachmentType_e::color, textureDescriptor texDesc = textureDescriptor())
	{
		this->sampleCount = sampleCount;
		this->attachmentType = attachmentType;
		attachmentFormat = 0;
		attachmentHandle = 0;
		layers = 0;

		this->dimensions = texDesc.dimensions;
		this->channels = texDesc.channels;
		this->format = texDesc.format;
		this->bitsPerPixel = texDesc.bitsPerPixel;

		this->currentMipmapLevel = texDesc.currentMipmapLevel;
		this->mipmapLevels = texDesc.mipmapLevels;
		this->border = texDesc.border;
		this->xOffset = texDesc.xOffset;
		this->yOffset = texDesc.yOffset;

		this->internalFormat = texDesc.internalFormat;
		this->target = texDesc.target;
		this->dataType = texDesc.dataType;

		this->minFilterSetting = texDesc.minFilterSetting;
		this->magFilterSetting = texDesc.magFilterSetting;
		this->wrapSSetting = texDesc.wrapSSetting;
		this->wrapTSetting = texDesc.wrapTSetting;
		this->wrapRSetting = texDesc.wrapRSetting;
	}

	GLuint				layers;
	GLuint				sampleCount;
	GLenum				attachmentFormat;
	GLuint				attachmentHandle;
	attachmentType_e	attachmentType;
};

class frameBuffer
{
public:

	//have this inherit from texture later on!
	class attachment_t : public texture
	{
	public:

		attachment_t(std::string uniformName = "defaultTexture", FBODescriptor FBODesc = FBODescriptor())
		{
			this->uniformName = uniformName;
			this->handle = 0;
			this->texType = texType;
			isResident = false;

			this->data = nullptr;			

			attachmentHandle = 0;
			//need a better system to this redundant bullshit
			this->texDesc = (textureDescriptor)FBODesc;
			this->FBODesc = FBODesc;

			glCreateTextures(this->FBODesc.target, 1, &handle);
			glBindTexture(this->FBODesc.target, handle);
			switch (this->FBODesc.target)
			{
			case GL_TEXTURE_2D_MULTISAMPLE:
			{

				glTextureStorage2DMultisample(handle, this->FBODesc.sampleCount, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, true);
				break;
			}

			case GL_TEXTURE_2D:
			{
				//parse internal format as bits per pixel
				glTexImage2D(this->FBODesc.target, this->FBODesc.currentMipmapLevel, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, this->FBODesc.border, this->FBODesc.format, this->FBODesc.dataType, nullptr);
				break;
			}

			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_3D:
			{
				TinyExtender::glTexImage3D(this->FBODesc.target, this->FBODesc.currentMipmapLevel, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, this->FBODesc.dimensions.z, this->FBODesc.border, static_cast<GLenum>(this->FBODesc.format), static_cast<GLenum>(this->FBODesc.dataType), nullptr);
				break;
			}
			default: break;
			}

			glTexParameteri(this->FBODesc.target, GL_TEXTURE_MIN_FILTER, this->FBODesc.minFilterSetting);
			glTexParameteri(this->FBODesc.target, GL_TEXTURE_MAG_FILTER, this->FBODesc.magFilterSetting);
			glTexParameteri(this->FBODesc.target, GL_TEXTURE_WRAP_S, this->FBODesc.wrapSSetting);
			glTexParameteri(this->FBODesc.target, GL_TEXTURE_WRAP_T, this->FBODesc.wrapTSetting);
			if(this->FBODesc.mipmapLevels > 0)
			{
				glGenerateMipmap(this->FBODesc.target);
			}
			UnbindTexture();
		}

		void Initialize(GLenum attachmentFormat)
		{
			FBODesc.attachmentFormat = attachmentFormat;
			if (FBODesc.layers > 0)
			{
				for (size_t iter = 0; iter < FBODesc.layers; iter++)
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + iter, handle, 0, (GLint)iter);
				}
			}
			else
			{

				glFramebufferTexture(GL_FRAMEBUFFER, attachmentFormat, handle, FBODesc.currentMipmapLevel);
			}

			switch(attachmentFormat)
			{
			case GL_DEPTH_ATTACHMENT:
			{
				SetReadMode(FBODescriptor::attachmentType_e::depth);
				break;
			}
			
			case GL_STENCIL_ATTACHMENT:
			{
				SetReadMode(FBODescriptor::attachmentType_e::stencil);
				break;
			}

			case GL_DEPTH_STENCIL_ATTACHMENT:
			{
				SetReadMode(FBODescriptor::attachmentType_e::stencil);
				break;
			}

			default:
			{
				break;
			}
			}
		}

		void Resize(glm::ivec2 newSize, bool unbind = true)
		{
			texDesc.dimensions = glm::ivec3(newSize, 1);
			FBODesc.dimensions = glm::ivec3(newSize, 1);

			switch (FBODesc.target)
			{
			case GL_TEXTURE_2D_MULTISAMPLE:
				{
					BindTexture();
					glDeleteTextures(1, &handle);
					glCreateTextures(FBODesc.target, 1, &handle);
					BindTexture();
					glTextureStorage2DMultisample(handle, this->FBODesc.sampleCount, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, true);
					UnbindTexture();
					break;
				}

			case GL_TEXTURE_2D:
				{
					BindTexture();
					glTexImage2D(FBODesc.target, FBODesc.currentMipmapLevel, FBODesc.internalFormat, FBODesc.dimensions.x, FBODesc.dimensions.y, FBODesc.border, FBODesc.format, FBODesc.dataType, nullptr);
					UnbindTexture();
					break;
				}

			case GL_TEXTURE_3D:
			case GL_TEXTURE_2D_ARRAY:
				{
					BindTexture();
					//glTexImage3D(this->FBODesc.target, this->FBODesc.currentMipmapLevel, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, this->FBODesc.dimensions.z, this->FBODesc.border, this->FBODesc.format, this->FBODesc.dataType, nullptr);
					UnbindTexture();
					break;
				}
			default: break;
			}
		}

		void Resize(glm::ivec3 newSize, bool unbind = true)
		{
			texDesc.dimensions = newSize;
			FBODesc.dimensions = newSize;

			switch (FBODesc.target)
			{
				case GL_TEXTURE_2D_MULTISAMPLE:
				{
					BindTexture();
					glDeleteTextures(1, &handle);
					glCreateTextures(FBODesc.target, 1, &handle);
					BindTexture();
					glTextureStorage2DMultisample(handle, this->FBODesc.sampleCount, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, true);
					UnbindTexture();
					break;
				}

				case GL_TEXTURE_2D:
				{
					BindTexture();
					glTexImage2D(FBODesc.target, FBODesc.currentMipmapLevel, FBODesc.internalFormat, FBODesc.dimensions.x, FBODesc.dimensions.y, FBODesc.border, FBODesc.format, FBODesc.dataType, nullptr);
					UnbindTexture();
					break;
				}

				case GL_TEXTURE_3D:
				case GL_TEXTURE_2D_ARRAY:
				{
					BindTexture();
					//glTexImage3D(this->FBODesc.target, this->FBODesc.currentMipmapLevel, this->FBODesc.internalFormat, this->FBODesc.dimensions.x, this->FBODesc.dimensions.y, this->FBODesc.dimensions.z, this->FBODesc.border, this->FBODesc.format, this->FBODesc.dataType, nullptr);
					UnbindTexture();
					break;
				}
			default: break;
			}
		}

		void SetReadMode(FBODescriptor::attachmentType_e attachmentType)
		{
			BindTexture();
			FBODesc.attachmentType = attachmentType;

			switch (FBODesc.attachmentType)
			{
				/*case FBODescriptor::attachmentType_e::depth:
				{
					glTexParameteri(FBODesc.target, gl_depth_texture_mode, GL_DEPTH_COMPONENT); //deprecated
					break;
				}*/

				case FBODescriptor::attachmentType_e::stencil:
				case FBODescriptor::attachmentType_e::depthAndStencil:
				{
					glTexParameteri(FBODesc.target, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
					break;
				}
			default: break;;
			}
			
			UnbindTexture();
		}

		void Draw() const
		{
			//if the current framebuffer is not this one then bind it
			switch (FBODesc.attachmentType)
			{
			case FBODescriptor::attachmentType_e::stencil:
			case FBODescriptor::attachmentType_e::depth:
			case FBODescriptor::attachmentType_e::depthAndStencil:
			{
				//if non-color, draw to GL_NONE
				GLenum attachment = GL_NONE;
				glDrawBuffers(1, &attachment);
				break;
			}

			default:
			{
				glDrawBuffers(1, &FBODesc.attachmentFormat);
				break;
			}
			}
		}

	public:
		GLuint			attachmentHandle;
		FBODescriptor	FBODesc;
	};

	frameBuffer()
	{
		bufferHandle = 0;	
	}

	void Initialize()
	{
		glGenFramebuffers(1, &bufferHandle);
		//glBindFramebuffer(gl_framebuffer, bufferHandle);
	}

	void Bind(GLenum target = GL_FRAMEBUFFER)
	{
		glBindFramebuffer(target, bufferHandle);
	}

	static void Unbind(GLenum target = GL_FRAMEBUFFER)
	{
		glBindFramebuffer(target, 0);
	}

	void DrawAll()
	{
		std::vector<GLenum> allImages;
		for (auto iter : attachments)
		{
			switch (iter.second.FBODesc.attachmentType)
			{
			case FBODescriptor::attachmentType_e::stencil:
			case FBODescriptor::attachmentType_e::depth:
			case FBODescriptor::attachmentType_e::depthAndStencil:
				{
					//if non-color, draw to GL_NONE
					allImages.push_back(GL_NONE);
					break;
				}

				default:
				{
					allImages.push_back(iter.second.FBODesc.attachmentFormat);
					break;
				}
			}
		}

		glDrawBuffers(allImages.size(), allImages.data());
	}

	void DrawDepth()
	{
		GLuint test = GL_DEPTH_ATTACHMENT;
		glDrawBuffers(1, &test);
	}

	void DrawMultiple(const char* name)
	{

	}

	void Resize(glm::ivec3 newSize/*, bool unbind = true*/)
	{
		//resize the buffers
		for (auto val : attachments | std::views::values)
		{
			val.Resize(newSize);
		}
	}

	static void ClearTexture(const attachment_t& attachment, const float clearColor[4])
	{
		switch (attachment.FBODesc.attachmentType)
		{
		case FBODescriptor::attachmentType_e::color:
		{	
			glClearBufferfv(GL_COLOR, attachment.attachmentHandle, clearColor);
			break;
		}

		case FBODescriptor::attachmentType_e::depth:
		{
			glClearBufferfv(GL_DEPTH, 0, clearColor);
			break;
		}

		case FBODescriptor::attachmentType_e::stencil:
		{
			glClearBufferiv(GL_STENCIL, 0, (GLint*)&clearColor[0]);
			break;
		}

		case FBODescriptor::attachmentType_e::depthAndStencil:
		{
			//glClearBufferfv(GL_STENCIL, attachment->attachmentHandle, clearColor);
			glClearBufferfi(GL_DEPTH, attachment.attachmentHandle, clearColor[0], (GLint)clearColor[1]);
			break;
		}
		}
	}

	void AddAttachment(attachment_t attachment)
	{
		//if the current framebuffer is not this one then bind it
		int currentBuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentBuffer);
		if (currentBuffer != bufferHandle)
		{
			Bind();
		}

		switch (attachment.FBODesc.attachmentType)
		{
		case FBODescriptor::attachmentType_e::color:
		{
			attachment.attachmentHandle = colorAttachmentNum;
			attachment.Initialize(GL_COLOR_ATTACHMENT0 + colorAttachmentNum);
			colorAttachmentNum++;
			break;
		}

		case FBODescriptor::attachmentType_e::depth:
		{
			attachment.attachmentHandle = GL_DEPTH_ATTACHMENT;
			attachment.Initialize(GL_DEPTH_ATTACHMENT);
			break;
		}

		case FBODescriptor::attachmentType_e::stencil:
		{
			attachment.attachmentHandle = GL_STENCIL_ATTACHMENT;
			attachment.Initialize(GL_STENCIL_ATTACHMENT);
			break;
		}

		case FBODescriptor::attachmentType_e::depthAndStencil:
		{
			attachment.attachmentHandle = GL_DEPTH_STENCIL_ATTACHMENT;
			attachment.Initialize(GL_DEPTH_STENCIL_ATTACHMENT);
			break;
		}
		}

		attachments.insert({attachment.uniformName, attachment});
		//attachments.push_back(attachment);
		CheckStatus();
	}

	/*void AddDepth(glm::vec2 size)
	{
		glGenRenderbuffers(1, &depthHandle);
		glBindRenderbuffer(gl_renderbuffer, depthHandle);
		glRenderbufferStorage(gl_renderbuffer, GL_DEPTH_COMPONENT,size.x, size.y);
		glFramebufferRenderbuffer(gl_framebuffer, gl_depth_attachment, gl_renderbuffer, depthHandle);
		glBindRenderbuffer(gl_renderbuffer, 0);
	}*/

	bool CheckStatus()
	{
		//if the current framebuffer is not this one then bind it
		glFinish();
		int currentBuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentBuffer);
		if (currentBuffer != bufferHandle)
		{
			Bind();
		}
		GLenum err = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		//TODO use a lookup table to speed this up
		if (err != GL_FRAMEBUFFER_COMPLETE)
		{
			switch (err)
			{
			case GL_FRAMEBUFFER_UNDEFINED:
			{
				printf("framebuffer undefined \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			{
				printf("framebuffer incomplete attachment \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			{
				printf("framebuffer missing attachment \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			{
				printf("framebuffer incomplete draw buffer \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			{
				printf("framebuffer incomplete read buffer \n");
				break;
			}

			case GL_FRAMEBUFFER_UNSUPPORTED:
			{
				printf("framebuffer unsupported \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			{
				printf("framebuffer incomplete multisample \n");
				break;
			}

			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			{
				printf("framebuffer incomplete layer targets \n");
				break;
			}
			default: break;;
			}
			return false;
		}
		return true;
	}

	//ok we need a target, handle, etc.
	GLuint							bufferHandle;
	//std::vector<attachment_t*>		attachments;
	tsl::robin_map<std::string, attachment_t>	attachments;
	GLuint							colorAttachmentNum = 0;
	GLuint							depthHandle = 0;
};
#endif