#ifndef TRANSFORM_H
#define TRANSFORM_H

class transform_t
{
public:

	transform_t(bool isRoot = false)
	{
		//no parent
		//iterEvent = nullptr;
		if (!isRoot)
		{
			parent = worldRoot.get();
		}
		else
		{
			parent = nullptr;
		}

		hierarchyCount = 0;

		position = glm::vec4(0, 0, 0, 1);
		rotation = glm::quat();
		scale = glm::vec4(1);
		eulerAngles = glm::vec4(0);

		localPosition = glm::vec4(0, 0, 0, 1);
		localRotation = glm::quat();
		localScale = glm::vec4(0);
		localEulerAngles = glm::vec4(0);

		lossyScale = 1;

		right = glm::conjugate(rotation) * globalRight;
		forward = glm::conjugate(rotation) * globalForward;
		up = glm::conjugate(rotation) * globalUp;
		localRight = right;
		localForward = forward;
		localUp = up;

		localIndex = 0;

		//setup space matrices
		UpdateSpaceMatrices();
		UpdateDirections();

		mouseOver = false;

		gizmoOp = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::LOCAL;

		name = "worldRoot";
	}

	transform_t(transform_t* parent)
	{
		//no parent
		//iterEvent = nullptr;
		this->parent = parent;
		hierarchyCount = 0;

		position = glm::vec4(0, 0, 0, 1);
		rotation = glm::quat();
		scale = glm::vec4(1);
		eulerAngles = glm::vec4(0);

		localPosition = glm::vec4(0, 0, 0, 1);
		localRotation = glm::quat();
		localScale = glm::vec4(1);
		localEulerAngles = glm::vec4(0);

		lossyScale = 1;

		if (parent != nullptr)
		{
			parent->children.push_back(this);

			//reset trimmed tree
			IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);
		}

		//setup space matrices
		UpdateDirections();
		//GetPathToRoot(this);

		gizmoOp = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::LOCAL;
	}

	~transform_t()
	{
		//find the link in both the local and global lists it is connected to
		//parent->children.erase(parent->children.begin() + localIndex);
		//globalList[globalIndex].release(); //can't delete itself?
	}

	enum class space_t
	{
		local,
		world
	};

	//setters and getters

	//global
	glm::vec4 GetPosition() { return position; }
	glm::quat GetRotation() { return rotation; }
	glm::vec4 GetScale() { return scale; }
	glm::vec4 GetEulerAngles() { return eulerAngles; }

	void SetPosition(glm::vec4 position)
	{
		this->position = position;
		transform = glm::mat4(up, right, forward, position);

		IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);

		//UpdateWorldPRS();
		UpdateLocalPRS();
	}
	void SetRotation(glm::quat rotation)
	{
		this->rotation = rotation;
		localRotation = rotation * glm::toQuat(transform);

		UpdateLocalPRS();
		UpdateDirections();
	}
	void SetScale(glm::vec4 scale)
	{
		this->scale = scale;
		this->localScale = scale * transform;
		
		UpdateLocalPRS();
	}
	void SetEulerAngles(glm::vec4 eulerAngles)
	{
		this->eulerAngles = eulerAngles;
		//set local euler angles

		//then update rotations

		IterateThroughChildren();
	}

	//local 
	glm::vec3 GetLocalPosition() { return localPosition; }
	glm::quat GetLocalRotation() { return localRotation; }
	glm::vec3 GetLocalScale() { return scale; }
	glm::vec3 GetLocalEulerAngles() { return localEulerAngles; }

	void SetLocalPosition(glm::vec4 localPosition)
	{
		this->localPosition = localPosition;
		//set global position
/*
		position = localPosition * localToWorldMatrix;

		UpdateWorldPRS();*/
	}
	void SetLocalRotation(glm::quat localRotation)
	{
		this->localRotation = localRotation;
		//set global rotation
		rotation = localRotation * glm::toQuat(transform);
		UpdateWorldPRS();
		UpdateDirections();
	}
	void SetLocalScale(glm::vec4 localScale)
	{
		this->localScale = localScale;
		//set global scale
		this->scale = localScale * localToWorldMatrix;
		UpdateWorldPRS();
	}
	void SetLocalEulerAngles(glm::vec4 localEulerAngles)
	{
		this->localEulerAngles = localEulerAngles;
	}

	void UpdateDirections()
	{
		right = glm::conjugate(rotation) * globalRight;
		forward = glm::conjugate(rotation) * globalForward;
		up = glm::conjugate(rotation) * globalUp;

		localRight = glm::conjugate(localRotation) * right;
		localForward = glm::conjugate(localRotation) * forward;
		localUp = glm::conjugate(localRotation) * up;
	}

	//if world position has changed (only call in setters/getters)
	void UpdateLocalPRS()
	{
		//localPosition = position * transform;
		
		//localRotation = rotation * glm::toQuat(worldToLocalMatrix);
		//localScale = scale * worldToLocalMatrix;
	}

	//if local position has changed
	void UpdateWorldPRS()
	{
		
		
		//rotation = localRotation * glm::toQuat(localToWorldMatrix);
		//scale = localScale * localToWorldMatrix;
	}

	static const std::unique_ptr<transform_t> worldRoot; //or just make these children or root

	//internals
	std::vector<transform_t*> children; //should only children have unique pointers?

	std::string name;

	transform_t* parent;
	unsigned int hierarchyCount;
	unsigned int localIndex;

	//global
	glm::vec4 position;		//hmm. operator overloading to automatically change local position when global has been changed?
	glm::quat rotation;
	glm::vec4 scale;
	glm::vec4 eulerAngles;

	//local members
	glm::vec4 localPosition;
	glm::quat localRotation;
	glm::vec4 localScale;
	glm::vec4 localEulerAngles;

	//conversion matrices
	glm::mat4 localToWorldMatrix;// change when P/S/R has changed? when to change? assuming this has a parent?
	glm::mat4 transform;

	//angles
	static const glm::vec4 globalRight;// = glm::vec3(1, 0, 0);
	static const glm::vec4 globalForward;// = glm::vec3(0, 0, 1);
	static const glm::vec4 globalUp;// = glm::vec3(0, 1, 0);

	glm::vec4 right;
	glm::vec4 forward;
	glm::vec4 up;

	glm::vec4 localRight;
	glm::vec4 localForward;
	glm::vec4 localUp;

	float lossyScale;//global scale of object as a scalar?

	bool mouseOver;
	ImGuizmo::OPERATION gizmoOp;// (ImGuizmo::ROTATE);
	ImGuizmo::MODE gizmoMode;// (ImGuizmo::LOCAL);

	//also keep a local vector of transform pointers so we don't have to iterate back down from root
	std::vector<const transform_t*> trimmedTree;

	void AddChild(transform_t* newChild)
	{
		//add this child to children then immediately go through its children to update matrices and locals
		children.push_back(newChild);
		newChild->parent = this;

		IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);
	}

	//unhook transform from current parent, set new parent, then set new transform matrices and local PRS
	void DetachSelf(transform_t* newParent = nullptr)
	{
		if (newParent == nullptr)
		{
			newParent = worldRoot.get();
		}

		//get where this node belongs in parent's tree
		localIndex = GetSiblingIndex();

		//delete this child from parent
		parent->children.erase(parent->children.begin() + localIndex);

		//change pointer to parent
		parent = newParent;

		//now move the new child into newparent's children
		parent->children.push_back(this);

		//reset the local index
		localIndex = GetSiblingIndex();

		//iterate though all of its children and update space matrices?
		//reset trimmed tree
		IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);
	}

	//detach all immediate children
	void DetachChildren()
	{
		//just step through the top most layer of children
		for (auto& iter : children)
		{
			//each immediate child is now child of world root
			iter->DetachSelf(worldRoot.get());
		}

		//this should remove all the empty/useless pointers
		children.clear();
	}

	void DetachChild(transform_t* toBeDetached)
	{
		for (auto& iter : children)
		{
			if (iter == toBeDetached)
			{
				iter->DetachSelf(worldRoot.get());
			}
		}
	}

	transform_t* GetChild(size_t childIndex)
	{
		//we just need to return a reference
		return children[childIndex];
	}

	int GetSiblingIndex()
	{
		int index = 0;
		for (auto iter : children)
		{
			if (iter == this)
			{
				return index;
			}
			index++;
		}

		return -1;
	}

	//set sibling index
	void SetSiblingIndex(size_t newIndex)
	{
		//swap where I currently am in the parent list and where I want to be
		std::iter_swap(parent->children.begin() + localIndex, parent->children.begin() + newIndex);
		localIndex = newIndex;
	}

	//I don't remember why these were here tbh
	void SetAsFirstSibling()
	{
		//just swap for now. figure out how to re-order later
		std::iter_swap(parent->children.begin(), parent->children.begin() + localIndex);
		localIndex = 0;
	}

	void SetAsLastSibling()
	{
		//just swap for now
		std::iter_swap(parent->children.end(), parent->children.begin() + localIndex);
		localIndex = parent->children.size() - 1;
	}

	void SetParent(transform_t* newParent)
	{
		//if this already has a parent then we need to decouple it from previous parent
		if (parent != nullptr && parent != worldRoot.get())
		{
			//transfer pointer ownership
			DetachSelf(newParent);
		}
		else
		{
			newParent->AddChild(this);
		}

		trimmedTree.clear();
		GetPathToRoot(this);
	}

	bool IsChildOf(transform_t* parent)
	{
		for (auto& iter : parent->children)
		{
			//if the iter is also pointing to this object then return the index
			if (iter == this)
			{
				return true;
			}
		}

		return false;
	}
	
	void IterateThroughChildren()
	{
		//for every child, set their callback to whatever is passed in and call it
		for (size_t iter = 0; iter < children.size(); iter++)
		{
			//bind the function pointer to the current child iter
			UpdateSpaceAndPRS();
			children[iter]->IterateThroughChildren();
		}
	}

	/*void IterateThroughChildren(/ *IterFunction pFun* /)
	{
		//for every child, set their callback to whatever is passed in and call it
		for (size_t iter = 0; iter < children.size(); iter++)
		{
			/ *iterEvent = std::bind(pFun, children[iter], _1);
			//bind the function pointer to the current child iter
			if (iterEvent != nullptr)
			{
				//call the event and then use recursion				
				iterEvent(children[iter]); //there is a chance that it only works for the original node, resetting root for this object over and over
			}* /

			//reset the callback later?
			children[iter]->IterateThroughChildren(/ *pFun* /);
		}
	}*/

	//iterate to root... and add each transform to trimmedTree
	void GetPathToRoot(transform_t* node)
	{
		if (node->parent != worldRoot.get() && node->parent != nullptr)
		{
			trimmedTree.push_back(parent);
			GetPathToRoot(node->parent);
		}
	}

	void GenNewChildrenMatrices(transform_t* node)
	{
		node->UpdateSpaceMatrices();
	}

	void UpdateSpaceAndPRS()
	{
		UpdateSpaceMatrices();
	}

	//local to world first
	void UpdateSpaceMatrices()
	{
		localToWorldMatrix = glm::identity<glm::mat4>();
		glm::mat4 localISR = glm::identity<glm::mat4>(); //Identity, Scale and Rotation
		glm::mat4 globalISR = glm::identity<glm::mat4>();
		//assuming the transform has no real parent then just use the world axis
		if (parent == worldRoot.get() || parent == nullptr)
		{
			//assuming the parent is the world root which is an identity matrix
			//here it doesn't matter which space was changed, local and world would be the same
			localPosition = glm::vec4(0);
			localRotation = glm::quat();
			localScale = glm::vec4(1);

			localISR = glm::scale(localISR, glm::vec3(localScale.x, localScale.y, localScale.z)) * glm::toMat4(localRotation);
			localISR[3] = localPosition;

			//transpose ISR into Up, Right, Forward, Position to create LtW matrix
			UpdateDirections();

			transform = glm::mat4(up, right, forward, position);
			//localToWorldMatrix = glm::inverse(worldToLocalMatrix);
		}

		else
		{
			//ok now go back through the list of transforms to generate the space transformation matrices
			//get the up, right, forward, globalPosition of parent
			//parent->UpdateDirections();
			UpdateDirections();
			
			glm::mat4 blarg = parent->transform + transform;
			localPosition.w = 1.0f;
			blarg[3].w = 1.0f;
			glm::vec4 blarg2 = blarg * transform[3];
			blarg2.w = 1.0f;
			position = blarg2;
			transform = glm::mat4(up, right, forward, localPosition);
			//transform[3] = position;

			// = localToWorldMatrix[3];

		//	UpdateWorldPRS();
			//UpdateLocalPRS();
		}
	}

	//Sets the world space position and rotation of the Transform component.
	void SetWorldPositionAndRotation(glm::vec4 position, glm::quat rotation)
	{
		this->position = position;
		this->rotation = rotation;

		//TODO: also update the localToWorld matrices of the children.
		//also update the locals if parent != worldRoot;
	}

	//Moves the transform in the direction and distance of translation. -- 
	void Translate(float x, float y, float z)
	{
		//position += arguments in wold space
		position += glm::vec4(x, y, z, 0);
		localPosition = position * transform;

		IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);
	}

	//Moves the transform in the direction and distance of translation. -- 
	void Translate(float x, float y, float z, space_t relativeTo)
	{
		if (relativeTo == space_t::local)
		{
			localPosition += glm::vec4(x, y, z, 0);
			UpdateWorldPRS();
		}

		else
		{
			position += glm::vec4(x, y, z, 0);
			UpdateLocalPRS();
			//TODO: update local position, etc.
		}

	}

	//Moves the transform in the direction and distance of translation. -- 
	void Translate(float x, float y, float z, transform_t* relativeTo)
	{
		//position += relativeTo.position - glm::vec3(x, y, z)

		//...ok first get our position in relation to the argument?
		//or create a vec3 for translation values relative to argument?
		//make an augmented localToWorld?
		//or multiply the vec3 by arg's world-to-local and apply it to me. then update matrices?
	}

	//Moves the transform in the direction and distance of translation. -- in world space
	void Translate(glm::vec3 translation)
	{
		//move in global space?
		position += glm::vec4(translation.x, translation.y, translation.z, 0);
		localPosition = position;
		IterateThroughChildren(/*&transform_t::GenNewChildrenMatrices*/);
	}

	//Moves the transform in the direction and distance of translation. -- 
	void Translate(glm::vec3 translation, space_t relativeTo)
	{
		switch (relativeTo)
		{
		case space_t::local:
		{
			localPosition += glm::vec4(translation.x, translation.y, translation.z, 0);
			break;
		}

		case space_t::world:
		{
			position += glm::vec4(translation.x, translation.y, translation.z, 0);
			break;
		}
		}
	}

	//Use Transform.Rotate to rotate GameObjects in a variety of ways. The rotation is often provided as a Euler angle and not a Quaternion. --
	void Rotate(glm::vec3 eulerAngles, space_t relativeTo = space_t::local)
	{
		rotation = glm::rotate(rotation, eulerAngles);
	}

	//Use Transform.Rotate to rotate GameObjects in a variety of ways. The rotation is often provided as a Euler angle and not a Quaternion. --
	void Rotate(float xAngle, float yAngle, float zAngle, space_t relativeTo = space_t::local)
	{
		rotation = glm::rotate(rotation, glm::vec3(xAngle, yAngle, zAngle));
	}

	//Use Transform.Rotate to rotate GameObjects in a variety of ways. The rotation is often provided as a Euler angle and not a Quaternion. --
	void Rotate(glm::vec3 axis, float angle, space_t relativeTo = space_t::local)
	{
		if (relativeTo == space_t::local)
		{
			localRotation = glm::rotate(localRotation, angle, axis);
			//UpdateWorldPRS();
		}

		else
		{
			rotation = glm::rotate(rotation, angle, axis);
			//UpdateLocalPRS();
		}
		
		if (!children.empty())
		{
			//go through all children and update their matrices


		}


	}

	//Rotates the transform about axis passing through point in world coordinates by angle degrees. --
	void RotateAround(const glm::vec3& point, glm::vec3 axis, float angle)
	{

	}

	//Rotates the transform so the forward vector points at /target/'s current position.
	void LookAt(const transform_t* target)
	{
		rotation = glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(target->position.x, target->position.y, target->position.z), glm::vec3(up.x, up.y, up.z));
		//will also need to change local rotation for all of these, children, etc.
	}

	void LookAt(const transform_t* target, glm::vec3 worldUp = globalUp)
	{
		rotation = glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(target->position.x, target->position.y, target->position.z), glm::vec3(worldUp.x, worldUp.y, worldUp.z));
	}

	void LookAt(const glm::vec3& worldPosition)
	{
		rotation = glm::quatLookAt(glm::vec3(worldPosition.x, worldPosition.y, worldPosition.z), glm::vec3(up.x, up.y, up.z));
	}

	void LookAt(const glm::vec3& worldPosition, glm::vec3 worldUp = globalUp)
	{
		rotation = glm::toQuat(glm::lookAt(glm::vec3(position.x, position.y, position.z), worldPosition, worldUp));
	}

	/*
Find	Finds a child by n and returns it. -- need to add a string to each transform? string name? hashtable?

TransformDirection	Transforms direction from local space to world space.
TransformPoint	Transforms position from local space to world space.
TransformVector	Transforms vector from local space to world space.

InverseTransformDirection	Transforms a direction from world space to local space. The opposite of Transform.TransformDirection.
InverseTransformPoint	Transforms position from world space to local space.
InverseTransformVector	Transforms a vector from world space to local space. The opposite of Transform.TransformVector.
	*/
};

//have a storage class to hold all of the transforms
struct transformContainer
{
	//add new transform
	transform_t* AddTransform()
	{
		globalList.push_back(std::make_unique<transform_t>());
		return globalList[globalList.size() - 1].get();
	}

	transform_t* AddTransform(transform_t* toCopy)
	{
		globalList.push_back(std::make_unique<transform_t>(toCopy));
	}

	transform_t* AddTransform(glm::vec4 position, glm::vec4 scale, glm::vec4 eulerRotation)
	{
		globalList.push_back(std::make_unique<transform_t>());
		transform_t* temp = globalList[globalList.size() - 1].get();
		temp->SetPosition(position);
		temp->SetScale(scale);
		temp->SetEulerAngles(eulerRotation);
		//no need to delete this pointer
	}

	//delete transform 
	void RemoveTransform(transform_t* toRemove)
	{
		for (size_t iter = 0; iter < globalList.size(); iter++)
		{
			if (globalList[iter].get() == toRemove)
			{
				transform_t* removal = globalList[iter].release();
				delete removal;
				globalList.erase(globalList.begin() + iter);
			}
		}
	}

	std::vector<std::unique_ptr<transform_t>> globalList;
};

const glm::vec4 transform_t::globalRight = glm::vec4(1, 0, 0, 1);
const glm::vec4 transform_t::globalForward = glm::vec4(0, 0, -1, 1);
const glm::vec4 transform_t::globalUp = glm::vec4(0, 1, 0, 1);

const std::unique_ptr<transform_t> transform_t::worldRoot = std::make_unique<transform_t>(true);
//std::vector<std::unique_ptr<transform_t>> transform_t::globalList = std::vector<std::unique_ptr<transform_t>>();

/*

namespace glm
{
	class vec4
	{
		using addEvent_t = std::function<void()>;
		addEvent_t addEvent = nullptr;


		vec4& operator=(const vec4& other)
		{
			xyzw = other;
			if(addEvent != nullptr)
			{
				addEvent();
			}
		}
	};
}

void updateLocal()
{
	//adjust localPosition when position is changed
}

vec4 position = glm::vec4(0);
position.addEvent = updateLocal;


*/

#endif