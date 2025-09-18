#pragma once

namespace raycast
{
    struct ray_t
    {
        public:

			ray_t()
			{
				start = glm::vec3(0);
				end = glm::vec3(0);
				direction = glm::vec3(0);
				distance = 0;

		}

        glm::vec3 start{};
        glm::vec3 end{};
        glm::vec3 direction{};
        float distance;
    };

    struct result
    {
	public:
		result()
		{
			hit = false;
		}

        
        bool hit;
    };

	//check against Plane


	//check against Quad

	//check against Triangle


    inline result CastRay(const ray_t& inRay, const grid& target, transform_t& inTrans)
    {
	   //in the future just go through all models loaded in the player view frustum. (use a BVH?)
		bool hit = false;
		float u, v, w = 0.0f;

		for (unsigned int vertexIter = 2; vertexIter < target.vertices.size(); vertexIter++)
		{
			//ok make the triangle  and apply the raycast math
			glm::vec3 P0 = target.vertices[vertexIter - 2].position;
			glm::vec3 P1 = target.vertices[vertexIter - 1].position;
			glm::vec3 P2 = target.vertices[vertexIter].position;

			//FROM THE HOLY ORANGE BOOK
			glm::vec3 PQ = inRay.start - inRay.end;
			glm::vec3 PA = P0 - inRay.start;
			glm::vec3 PB = P1 - inRay.start;
			glm::vec3 PC = P2 - inRay.start;

			glm::vec3 m = glm::cross(PQ, inRay.start);
			u = glm::dot(PQ, glm::cross(PC, PB)) + glm::dot(m, PC - PB);
			v = glm::dot(PQ, glm::cross(PA, PC)) + glm::dot(m, PA - PC);
			w = glm::dot(PQ, glm::cross(PB, PA)) + glm::dot(m, PB - PA);
			if (u < 0.0f)
			{
				printf("poop U \n");
			}
			if (v < 0.0f)
			{
				printf("poop V \n");
			}
			if (w < 0.0f)
			{
				printf("poop W \n");
			}

			float denom = 1.0f / (u + v + w);
			u *= denom;
			v *= denom;
			w *= denom;


/*
			glm::vec3 N = glm::cross(P1 - P0, P2 - P0);
			float d = glm::dot(-N, P0);
			glm::vec4 L = glm::vec4(N.x, N.y, N.z, d);

			float intersectPoint1 = glm::dot(L, glm::vec4(inRay.start, 1));
			float intersectPoint2 = glm::dot(L, glm::vec4(inRay.direction, 1));*/



			float t = 0;// (intersectPoint1 / intersectPoint2);

			//glm::vec3 point = inRay.start + (t * inRay.direction);

			//ok now what to look for?
			if (t > 0) //how the fuck do i tell if there has been a collision?
			{
				hit = true;
				break;
			}
		}

		result res;
		res.hit = hit;
		return res;
    }

    inline result CastRay(const ray_t& inRay, model_t& target, const transform_t& inTrans)
    {
        //uhh go through every triangle loaded to determine what is hit. i don't have collision volumes...so triangles

        //in the future just go through all models loaded in the player view frustum
        bool hit = false;
		//glm::mat4 position = glm::translate(glm::mat4(1.0f), glm::vec3(inTrans.position));
		//glm::mat4 rotation = glm::toMat4(inTrans.rotation);
		///glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(inTrans.scale));
		//transform the position via PRS transforms
		//glm::mat4 PRS = position * rotation * scale;


        for (auto meshIter : target.meshes)
        {
            for (unsigned int vertexIter = 2; vertexIter < meshIter.vertices.size(); vertexIter++)
            {
                //ok make the triangle  and apply the raycast math
                glm::vec3 P0 = meshIter.vertices[vertexIter - 2].position;
                glm::vec3 P1 = meshIter.vertices[vertexIter - 1].position;
                glm::vec3 P2 = meshIter.vertices[vertexIter].position;

                glm::vec3 N = glm::cross(P1 - P0, P2 - P0);
                float d = glm::dot(-N, P0);
                glm::vec4 L = glm::vec4(N.x, N.y, N.z, d);

                float intersectPoint1 = glm::dot(L, glm::vec4(inRay.start, 1));
                float intersectPoint2 = glm::dot(L, glm::vec4(inRay.direction, 1));
                

                float t = -(intersectPoint1 / intersectPoint2);

                glm::vec3 point = inRay.start + (t * inRay.direction);

                //ok now what to look for?
                if(t > 0) //how the fuck do i tell if there has been a collision?
                {
                    hit = true;
                }
            }
        }
		result res;
		res.hit = hit;
        return res;
    }

	result RayFromMouse(camera_t& inCamera, glm::vec2 mousePosition, model_t& target, transform_t& inTrans)
	{
        //translate mouse position from screen space into world space
        //then fire a ray from that via the cameras forward vector as direction
        
        auto invMat = glm::inverse(inCamera.view * inCamera.projection);

        //
        glm::vec4 rayStart = invMat * glm::vec4(mousePosition.x, mousePosition.y, 0.0, 1.0f);
        glm::vec4 rayEnd = inCamera.view * glm::vec4(mousePosition.x, mousePosition.y, 1.0f, 1.0f);

        glm::vec4 rayVel = glm::normalize(rayEnd - rayStart);

		ray_t newRay;
		newRay.start = rayStart;
		newRay.end = rayEnd;
		newRay.direction = rayVel;

		return CastRay(newRay, target, inTrans);
	}

	result RayFromMouse(camera_t& inCamera, glm::vec2 mousePosition, const grid& target, transform_t inTrans)
	{
		glm::vec2 normMousePos = glm::normalize(mousePosition);
		glm::vec4 clipCoord = glm::vec4(normMousePos.x, normMousePos.y, -1000, 1000);

		//what projection mode is it in? do we just force perspective?
		glm::mat4 invProj = inverse(inCamera.MakeProjection(camera_t::projection_t::perspective));
		glm::mat4 invView = inverse(inCamera.MakeView(camera_t::projection_t::perspective));

		glm::vec4 eyeCoords = invProj * clipCoord;	//moves clip coordinates into camera coordinates
		glm::vec4 rayStart = invView * eyeCoords;	//from camera coords into world coordinates (via inverse view matrix)
		glm::vec3 rayDirection = glm::vec3(glm::normalize(rayStart));	//normalize start ray to set ray direction?


		ray_t newRay;
		newRay.start = rayStart;
		newRay.direction = rayDirection;

		return CastRay(newRay, target, inTrans);
	}
}



