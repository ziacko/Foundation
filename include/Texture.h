#pragma once
//instead of one mega class of texture, just break it down into seperate 

struct textureDescriptor
{
	explicit textureDescriptor(const GLenum& target = GL_TEXTURE_2D, const GLenum& dataType = GL_UNSIGNED_BYTE,
		const GLenum& format = GL_RGBA, const GLint& internalFormat = GL_RGBA8,
		const GLenum& minFilterSetting = GL_LINEAR, const GLenum& magFilterSetting = GL_LINEAR,
		const GLenum& wrapSSetting = GL_REPEAT, const GLenum& wrapTSetting = GL_REPEAT, const GLenum& wrapRSetting = GL_CLAMP_TO_EDGE,
		const GLenum& access = GL_READ_WRITE)
	{
		dimensions = glm::vec3(defaultWindowSize.x, defaultWindowSize.y, 1);
		this->channels = 4;
		this->format = format;
		this->bitsPerPixel = 8;

		this->currentMipmapLevel = 0;
		this->mipmapLevels = 0;
		this->border = 0;
		this->xOffset = 0;
		this->yOffset = 0;

		this->internalFormat = internalFormat;
		this->target = target;
		this->dataType = dataType;

		this->minFilterSetting = minFilterSetting;
		this->magFilterSetting = magFilterSetting;
		this->wrapSSetting = wrapSSetting;
		this->wrapTSetting = wrapTSetting;
		this->wrapRSetting = wrapRSetting;

		this->isImmutable = false;
		this->access = access;
	}

	//size and pixel depth density settings
	glm::ivec3		dimensions;
	GLint			channels;
	GLenum			format;

	//texture formats and types
	GLint			internalFormat;
	GLenum			target;
	GLint			currentMipmapLevel;
	GLint			mipmapLevels;
	GLint			border;
	GLenum			dataType;
	GLint			xOffset;
	GLint			yOffset;

	//filtering settings
	GLenum			minFilterSetting;
	GLenum			magFilterSetting;
	GLenum			wrapSSetting;
	GLenum			wrapTSetting;
	GLenum			wrapRSetting;

	GLuint			bitsPerPixel;
	GLenum			access;

	bool			isImmutable;
};

class texture
{
public:

	enum class textureType_t
	{
		image,
		diffuse,
		normal,
		specular,
		height,
		roughness,
		metallic,
		ambientOcclusion,
		albedo
	};

	explicit texture(const std::string& path = "textures/earth_diffuse.tga", const textureType_t& texType = textureType_t::image,
	                 const std::string& uniformName = "defaultTexture", const textureDescriptor& texDescriptor = textureDescriptor())
	{
		this->path = path;
		this->uniformName = uniformName;
		this->handle = 0;
		this->texType = texType;

		this->texDesc = texDescriptor;

		isResident = false;
		residentHandle = 0;
		this->data = nullptr;
		uniformHandle = 0;
	}
	
	virtual ~texture() = default;

	void SetActive() const
	{
		glBindTextureUnit(handle, handle);
	}

	virtual void SetActive(const GLuint& texUnit) const
	{
		glBindTextureUnit(texUnit, handle);
	}
	
	virtual void GetUniformLocation(const GLuint& programHandle)
	{
		uniformHandle = glGetUniformLocation(programHandle, uniformName.c_str());
		glUniform1i(uniformHandle, handle);

		SetActive();
	}

	virtual void BindTexture() const
	{
		glBindTexture(texDesc.target, handle);
	}

	void UnbindTexture() const
	{
		glBindTexture(texDesc.target, 0);
	}

	static void UnbindTexture(const GLenum& target)
	{
		glBindTexture(target, 0);
	}

	void BindAsImage(const GLuint& texUnit, const GLenum& access = GL_WRITE_ONLY, const bool& layered = false, const GLint& layer = 0) const
	{
		glBindImageTexture(texUnit, handle, texDesc.currentMipmapLevel, layered, layer, access, texDesc.internalFormat);
	}

	virtual void OverloadTextureUnit(const GLuint& texUnit) const
	{
		glActiveTexture(GL_TEXTURE0 + texUnit);
		glBindTexture(texDesc.target, handle);
	}

	void LoadTexture()
	{
		stbi_set_flip_vertically_on_load(true);

		const auto fullPath = ASSET_DIR + path;
		char* data = (char*)stbi_load(fullPath.c_str(), &texDesc.dimensions.x, &texDesc.dimensions.y, &texDesc.channels, 0);

		//if stbi fails then use gli instead. if that fails give up
		if (data == nullptr)
		{
			gli::texture tex = gli::load(path);
			if (!tex.empty())
			{
				gliLoad(tex);
			}
			else
			{
				printf("couldn't load texture: %s \n", fullPath.c_str());
				return;
			}
		}

		else
		{
			stbLoad(data);
		}
	}

	virtual void ReloadTexture(const std::string& path)
	{
		stbi_set_flip_vertically_on_load(true);
		const auto fullPath = ASSET_DIR + path;

		char* data = (char*)stbi_load(fullPath.c_str(), &texDesc.dimensions.x, &texDesc.dimensions.y, &texDesc.channels, 0);

		if (data != nullptr)
		{
			stbLoad(data, true);
		}

		else
		{

			const gli::texture tex = gli::load(path);

			if (tex.empty())
			{
				return;
			}
			gliLoad(tex);
		}
		SetPath(fullPath.c_str());
	}

	virtual void SetMinFilter(const GLenum& minFilterSetting)
	{
		switch (minFilterSetting)
		{
			case 0: texDesc.minFilterSetting = GL_LINEAR; break;
			case 1: texDesc.minFilterSetting = GL_NEAREST; break;
			case 2: texDesc.minFilterSetting = GL_NEAREST_MIPMAP_NEAREST; break;
			case 3: texDesc.minFilterSetting = GL_NEAREST_MIPMAP_LINEAR; break;
			case 4: texDesc.minFilterSetting = GL_LINEAR_MIPMAP_NEAREST; break;
			case 5: texDesc.minFilterSetting = GL_LINEAR_MIPMAP_LINEAR; break;
			default: break;
		}
		BindTexture();
		glTexParameteri(texDesc.target, GL_TEXTURE_MIN_FILTER, texDesc.minFilterSetting);
		//UnbindTexture();
	}

	virtual void SetMagFilter(const GLenum& magFilterSetting)
	{
		switch (magFilterSetting)
		{
			case 0: texDesc.magFilterSetting = GL_LINEAR; break;
			case 1: texDesc.magFilterSetting = GL_NEAREST; break;
			case 2: texDesc.magFilterSetting = GL_NEAREST_MIPMAP_NEAREST; break;
			case 3: texDesc.magFilterSetting = GL_NEAREST_MIPMAP_LINEAR; break;
			case 4: texDesc.magFilterSetting = GL_LINEAR_MIPMAP_NEAREST; break;
			case 5: texDesc.magFilterSetting = GL_LINEAR_MIPMAP_LINEAR; break;
			default: break;
		}

		BindTexture();
		glTexParameteri(texDesc.target, GL_TEXTURE_MAG_FILTER, texDesc.magFilterSetting);
		//UnbindTexture();
	}

	virtual void SetWrapS(const GLenum& wrapSetting)
	{
		texDesc.wrapSSetting = wrapSetting;
		
		BindTexture();
		glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_S, texDesc.wrapSSetting);
		glFinish();
		//UnbindTexture();
	}

	virtual void SetWrapT(const GLenum& wrapSetting)
	{
		texDesc.wrapTSetting = wrapSetting;

		BindTexture();
		glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_T, texDesc.wrapTSetting);
		//UnbindTexture();
	}

	virtual void SetWrapR(const GLenum& wrapSetting)
	{
		texDesc.wrapRSetting = wrapSetting;

		BindTexture();
		glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_R, texDesc.wrapRSetting);
		UnbindTexture();
	}

	virtual void SetPath(const char* newPath)
	{
		path = newPath;
	}

	virtual void SetTextureType(const textureType_t& newType)
	{
		this->texType = newType;
	}

	std::string GetFilePath() const
	{
		return path;
	}

	std::string GetUniformName() const
	{
		return uniformName;
	}

	unsigned int GetHandle() const
	{
		return handle;
	}

	GLuint64 GetResidentHandle() const
	{
		return residentHandle;
	}

	std::vector<float> GetPixels() const
	{
		const int bytes = texDesc.dimensions.x * texDesc.dimensions.y * 2;

		GLfloat* pixels = new GLfloat[bytes];

		glGetTexImage(texDesc.target, texDesc.currentMipmapLevel, texDesc.internalFormat, texDesc.dataType, pixels);

		std::vector<float> result;
		result.assign(pixels, pixels + bytes);
		delete pixels; //do some cleanup
		return result;
	}

	//copy another texture into itself. just 2D textures right now
	void Copy(const texture* otherTexture) const
	{
		glCopyImageSubData(otherTexture->handle, otherTexture->texDesc.target, otherTexture->texDesc.currentMipmapLevel, 0, 0, 0,
			handle, texDesc.target, texDesc.currentMipmapLevel, 0, 0, 0,
			texDesc.dimensions.x, texDesc.dimensions.y, texDesc.dimensions.z);
	}

	void ToggleResident()
	{
		isResident = !isResident;

		if (isResident)
		{
			glMakeTextureHandleResidentARB(residentHandle);
		}

		else
		{
			glMakeTextureHandleNonResidentARB(residentHandle);
		}

	}

//protected:
	GLuint				handle;
	std::string			path;
	GLuint				uniformHandle;
	std::string			uniformName;

	textureDescriptor	texDesc;
	char*				data;

	textureType_t		texType;

	GLuint64			residentHandle;
	bool				isResident;

private:

	enum class loadType_e
	{
		image,
		texture
	};

	void stbLoad(const char* data, const bool& reload = false)
	{
		switch (texDesc.channels)
		{
			case 1: texDesc.format = GL_R; break;
			case 2: texDesc.format = GL_RG; break;
			case 3: texDesc.format = GL_RGB; break;
			case 4: texDesc.format = GL_RGBA; break;
			default: break;
		}

		if(!reload)
		{
			glGenTextures(1, &handle);
		}
		
		glBindTexture(texDesc.target, handle);

		switch (texDesc.target)
		{
			case GL_TEXTURE_1D:
			case GL_TEXTURE_1D_ARRAY:
			{
				//TODO: implement 1D array init
				break;
			}

			case GL_TEXTURE_2D:
			{
				glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_S, texDesc.wrapSSetting);
				glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_T, texDesc.wrapTSetting);

				if(texDesc.isImmutable)
				{
					glTexStorage2D(texDesc.target, texDesc.mipmapLevels, texDesc.internalFormat, texDesc.dimensions.x, texDesc.dimensions.y);
					glTextureSubImage2D(handle, texDesc.currentMipmapLevel, texDesc.xOffset, texDesc.yOffset, texDesc.dimensions.x, texDesc.dimensions.y, texDesc.format, texDesc.dataType, data);
				}

				else
				{
					glTexImage2D(texDesc.target, texDesc.currentMipmapLevel, texDesc.internalFormat, texDesc.dimensions.x, texDesc.dimensions.y, texDesc.border, texDesc.format, texDesc.dataType, data);
				}

				if (texDesc.mipmapLevels > 0)
				{
					glGenerateMipmap(GL_TEXTURE_2D);
				}

				break;
			}

			case GL_TEXTURE_2D_ARRAY:
			{
					//TODO: implement 2D array system
				break;
			}

			case GL_TEXTURE_2D_MULTISAMPLE:
			{
				glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_S, texDesc.wrapSSetting);
				glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_T, texDesc.wrapTSetting);
				break;
			}

			case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			case GL_TEXTURE_3D:
			case GL_TEXTURE_BUFFER:
			case GL_TEXTURE_RECTANGLE:
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
			default: break;
		}

		if (texDesc.mipmapLevels > 0)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, texDesc.mipmapLevels);
			glTexParameteri(texDesc.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(texDesc.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		}

		else
		{			
			glTexParameteri(texDesc.target, GL_TEXTURE_MIN_FILTER, texDesc.minFilterSetting);
			glTexParameteri(texDesc.target, GL_TEXTURE_MAG_FILTER, texDesc.magFilterSetting);
		}

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso); //throws out an openGL error but works anyway. not sure how to fix
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
	}

	void gliLoad(gli::texture tex)
	{
		tex = gli::flip(tex);
		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const gliFormat = GL.translate(tex.format(), tex.swizzles());
		texDesc.target = GL.translate(tex.target());
		texDesc.mipmapLevels = (GLint)tex.levels();
		texDesc.currentMipmapLevel = 0;
		texDesc.internalFormat = gliFormat.Internal;
		texDesc.format = gliFormat.External;
		texDesc.dataType = gliFormat.Type;

		glm::vec3 res = tex.extent();
		texDesc.dimensions = res;

		bool compressed = gli::is_compressed(tex.format());

		glGenTextures(1, &handle);
		glBindTexture(texDesc.target, handle);

		switch (texDesc.target)
		{
		case GL_TEXTURE_1D:
		{
			break;
		}

		case GL_TEXTURE_1D_ARRAY:
		{
			break;
		}

		case GL_TEXTURE_2D:
		{
			glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_S, texDesc.wrapSSetting);
			glTexParameteri(texDesc.target, GL_TEXTURE_WRAP_T, texDesc.wrapTSetting);
			glTexParameteri(texDesc.target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(texDesc.target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(tex.levels() - 1));
			glTexParameteriv(texDesc.target, GL_TEXTURE_SWIZZLE_RGBA, &gliFormat.Swizzles[0]);

			for (unsigned int level = 0; level < tex.levels(); level++)
			{
				glm::tvec3<GLsizei> extents(tex.extent(level));
				if(compressed)
				{
					glTexStorage2D(texDesc.target, texDesc.currentMipmapLevel, texDesc.internalFormat, texDesc.dimensions.x, texDesc.dimensions.y);
					glCompressedTexSubImage2D(
						texDesc.target, static_cast<GLint>(level), 0, 0, extents.x, extents.y,
						texDesc.internalFormat, static_cast<GLsizei>(tex.size(level)), tex.data(0, 0, level));
				}

				else
				{
					texDesc.currentMipmapLevel = 0;
					glTexImage2D(texDesc.target, texDesc.currentMipmapLevel, texDesc.internalFormat, texDesc.dimensions.x, texDesc.dimensions.y, texDesc.border, texDesc.format, texDesc.dataType, tex.data(0, 0, level));
				}
			}
			break;
		}

		case GL_TEXTURE_2D_ARRAY:
		{
			break;
		}

		case GL_TEXTURE_2D_MULTISAMPLE:
		{
			break;
		}

		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		{
			break;
		}

		case GL_TEXTURE_3D:
		{
			break;
		}

		case GL_TEXTURE_BUFFER:
		{
			break;
		}

		case GL_TEXTURE_RECTANGLE:
		{
			break;
		}

		case GL_TEXTURE_CUBE_MAP:
		{
			break;
		}

		case GL_TEXTURE_CUBE_MAP_ARRAY:
		{
			break;
		}

		default:
		{

			break;
		}
		}
		glTexParameteri(texDesc.target, GL_TEXTURE_MIN_FILTER, texDesc.minFilterSetting);
		glTexParameteri(texDesc.target, GL_TEXTURE_MAG_FILTER, texDesc.magFilterSetting);

		UnbindTexture();
	}

	//are you a texture or an image?
	void ThrowIntoGL(loadType_e& loadTYpe, const bool& compressed)
	{

		//do the common work first then alter per type

		switch (loadTYpe)
		{
		case loadType_e::image:
			{
				break;
			}

		case loadType_e::texture:
			{
				break;
			}
		}



	}


	//ok let's maybe use a temp
};
