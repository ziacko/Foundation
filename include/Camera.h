#pragma once

class camera_t
{
public:

	enum class projection_e
	{
		perspective,
		orthographic,
	};

	glm::mat4			translation;
	glm::mat4			projection;
	glm::mat4			view;

	glm::vec2			mousePosition;
	glm::vec2			resolution;
	float				speed;
	float				fieldOfView;

	float				farPlane;
	float				nearPlane;
	projection_e		currentProjectionType;

	float				xSensitivity;
	float				ySensitivity;
	float				zSensitivity;

	float				yaw = 0.0f;
	float				pitch = 0.0f;
	float				roll = 0.0f;

	glm::vec4			right;
	glm::vec4			forward;
	glm::vec4			up;

	//glm::vec4 rotation;
	glm::vec3 position;
	glm::vec3 rotator; //roll, pitch, yaw
	glm::quat rotation;

	bool guiActive = false;

	static const glm::vec4 globalRight;// = glm::vec3(1, 0, 0);
	static const glm::vec4 globalForward;// = glm::vec3(0, 0, 1);
	static const glm::vec4 globalUp;// = glm::vec3(0, 1, 0);
	static const float		defaultOrthoNear;// = 0.01f;
	static const float		defaultOrthoFar;// = 100.0f;
	static const float		defaultPersNear;// = 15.0f;
	static const float		defaultPersFar;//= 1000.0f;

	explicit camera_t(const glm::vec2& resolution = defaultWindowSize, const float& speed = defaultCameraSpeed,
	                  const projection_e& type = projection_e::orthographic, const float& nearPlane = defaultNearPlane,
	                  const float& farPlane = defaultFarPlane, const float& fieldOfView = defaultFieldOfView)
	{
		this->farPlane = farPlane;
		this->nearPlane = nearPlane;
		this->fieldOfView = fieldOfView;
		this->speed = speed;
		this->currentProjectionType = type;
		this->translation = glm::mat4(1.0f);
		this->resolution = resolution;
		xSensitivity = 0.01f;
		ySensitivity = 0.01f;
		zSensitivity = 0.01f;
		mousePosition = glm::vec2(0.0f);

		(this->currentProjectionType == projection_e::orthographic)
			? this->projection = glm::ortho(0.0f, this->resolution.x, this->resolution.y,
			                                0.0f, this->nearPlane, this->farPlane)
			: this->projection = glm::perspective<float>(this->fieldOfView, this->resolution.x / this->resolution.y,
			                                             this->nearPlane, this->farPlane);

		if (currentProjectionType == projection_e::orthographic)
		{
			this->view = glm::mat4(1);
			this->view[3][2] = -5.0f;
		}

		else
		{
			this->view = glm::inverse(this->translation);
		}

		position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		rotation = glm::quat(glm::vec3(0.0f));
		rotator = glm::vec3(0.0f);

		up = globalUp;
		right = globalRight;
		forward = globalForward;
	}

	~camera_t() {}

	//copy constructor
	camera_t(const camera_t& other)
	{
		this->position = other.position;
		this->rotation = other.rotation;
		this->rotator = other.rotator;
		this->up = other.up;
		this->right = other.right;
		this->forward = other.forward;
		this->fieldOfView = other.fieldOfView;
		this->nearPlane = other.nearPlane;
		this->farPlane = other.farPlane;
		this->speed = other.speed;
		this->currentProjectionType = other.currentProjectionType;
		this->translation = other.translation;
		this->resolution = other.resolution;
		this->speed = other.speed;
		this->projection = other.projection;
		this->view = other.view;
		this->mousePosition = other.mousePosition;
		this->xSensitivity = other.xSensitivity;
		this->ySensitivity = other.ySensitivity;
		this->zSensitivity = other.zSensitivity;
	}

	void Update()
	{
		if (currentProjectionType == projection_e::perspective)
		{
			view = glm::eulerAngleXYZ(rotator.y, rotator.x, rotator.z);
			view = glm::translate(view, -position);
			rotation = glm::toQuat(view);
		}
		UpdateProjection();
		UpdateDirections();
	}

	void UpdateProjection()
	{
		if (currentProjectionType == projection_e::perspective)
		{
			if (resolution.x > 0 && resolution.y > 0)
			{
				projection = glm::perspective<float>(glm::radians(fieldOfView), resolution.x / resolution.y,
					nearPlane, farPlane);
			}
		}

		else 
		{
			projection = glm::ortho<float>(0.0f, resolution.x, resolution.y, 0.0f, defaultOrthoNear, defaultOrthoFar);
		}
	}

	glm::mat4 MakeProjection(const projection_e& projectionType) const
	{
		if (projectionType == projection_e::perspective)
		{
			return glm::perspective<float>(glm::radians(fieldOfView), resolution.x / resolution.y,
				nearPlane, farPlane);
		}

		else
		{
			return glm::ortho<float>(0.0f, resolution.x, resolution.y, 0.0, defaultOrthoNear, defaultOrthoFar);
		}
	}

	glm::mat4 MakeView(const projection_e& projectionType)
	{
		glm::mat4 outView = glm::mat4(1.0f);
		if (projectionType == projection_e::perspective)
		{
			outView = glm::eulerAngleXYZ(rotator.y, rotator.x, rotator.z);
			outView = glm::translate(outView, -position);
		}

		else
		{
			this->view = glm::mat4(1);
			this->view[3][2] = -5.0f;
		}

		return outView;
	}

	void ChangeProjection(projection_e newProjection)
	{
		currentProjectionType = newProjection;

		if (currentProjectionType == projection_e::perspective)
		{
			if (resolution.x > 0 && resolution.y > 0)
			{
				projection = glm::perspective<float>(glm::radians(fieldOfView), resolution.x / resolution.y,
					nearPlane, farPlane);
				view = glm::eulerAngleXYZ(rotator.y, rotator.x, rotator.z);
				view = glm::translate(view, -position);
				rotation = glm::toQuat(view);
			}
		}

		else
		{
			projection = glm::ortho<float>(0.0f, resolution.x, resolution.y, 0.0, defaultOrthoNear, defaultOrthoFar);
			this->view = glm::mat4(1);
			this->view[3][2] = -5.0f;
			this->translation = glm::mat4(1.0f);
		}
	}

	void UpdateDirections()
	{
		right = glm::conjugate(rotation) * globalRight;
		forward = glm::conjugate(rotation) * globalForward;
		up = glm::conjugate(rotation) * globalUp;
	}

	void Pitch(const float& pitchRadians)
	{
		rotator += glm::vec3(globalUp) * pitchRadians;
	}

	void Yaw(const float& yawRadians)
	{
		rotator += glm::vec3(globalRight) * yawRadians;
	}

	void Roll(const float& rollRadians)
	{
		rotator += glm::vec3(globalForward) * rollRadians;
	}

	glm::vec3 GetForward() const
	{
		return glm::conjugate(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 GetRight() const
	{
		return glm::conjugate(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::vec3 GetUp() const
	{
		return glm::conjugate(rotation) * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	void MoveForward(const float& movement, const float& deltaTime)
	{
		position += (GetForward() * movement) * (1 - deltaTime);
	}

	void MoveRight(const float& movement, const float& deltaTime)
	{
		position += (GetRight() * movement) * (1 - deltaTime);
	}

	void MoveUp(const float& movement, const float& deltaTime)
	{
		position += (GetUp() * movement) * (1 - deltaTime);
	}
};

const glm::vec4 camera_t::globalRight = glm::vec4(1, 0, 0, 1);
const glm::vec4 camera_t::globalForward = glm::vec4(0, 0, -1, 1);
const glm::vec4 camera_t::globalUp = glm::vec4(0, 1, 0, 1);

const float	camera_t::defaultOrthoNear = 0.01f;
const float	camera_t::defaultOrthoFar = 100.0f;
const float camera_t::defaultPersNear = 0.1f;
const float camera_t::defaultPersFar = 1000.0f;
