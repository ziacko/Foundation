#pragma once

#include <scene.h>
#include <cstdlib>
#include "UniformBuffer.h"

class dotProductScene : public scene
{
public:

	struct node_t
	{
		glm::vec2		position1 = glm::vec2(20, 20);
		glm::vec2		position2 = glm::vec2(80, 120);
		float			scaler = 10;
		int				flipper = 0;
	};

	dotProductScene(const char* windowName = "Ziyad Barakat's portfolio (dot product helper)",
		camera_t camera = camera_t(), const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, camera, shaderConfigPath){}

	~dotProductScene() {};

protected:

	bufferHandler_t<node_t>		nodes;

	void SetupVertexBuffer() override
	{
		defaultVertexBuffer = vertexBuffer_t(glm::vec2(defaultPayload.data.resolution / nodes.data.scaler));
	}

	void Draw() override
	{	
		//just draw twice. don't over-think it
		nodes.data.flipper = 0;
		nodes.Update();
		glUseProgram(this->programGLID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		nodes.data.flipper = 1;
		nodes.Update();
		glUseProgram(this->programGLID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		DrawGUI(window);
		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Update() override
	{
		manager->PollForEvents();
		if (lockedFrameRate > 0)
		{
			clock.UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			clock.UpdateClockAdaptive();
		}

		defaultPayload.data.totalFrames++;
		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());
		defaultPayload.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	void InitializeUniforms() override 
	{
		defaultPayload.data = defaultUniformBuffer(this->camera);
		glm::vec2 resolution = defaultPayload.data.resolution;
		glViewport(0, 0, resolution.x, resolution.y);
		defaultPayload.data.resolution = resolution;
		defaultPayload.data.projection = glm::ortho(0.0f, resolution.x, resolution.y, 0.0f, 0.01f, 10.0f);

		SetupVertexBuffer();
		SetupBuffer(gl_uniform_buffer, gl_dynamic_draw);

		SetupDefaultUniforms();
		nodes.Initialize(1);
	}

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene::BuildGUI(window, io);

		glm::vec2 resolution = defaultPayload.data.resolution;
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		draw_list->AddLine(ImVec2(0,0), ImVec2(nodes.data.position1.x, nodes.data.position1.y), ImColor(255, 0, 255));
		draw_list->AddLine(ImVec2(0, 0), ImVec2(nodes.data.position2.x, nodes.data.position2.y), ImColor(255, 0, 255));
		draw_list->AddLine(ImVec2(nodes.data.position1.x, nodes.data.position1.y), ImVec2(nodes.data.position2.x, nodes.data.position2.y), ImColor(0, 255, 0));

		ImGui::Separator();
		ImGui::SliderFloat("node 1 X", &nodes.data.position1.x, 0, resolution.x - (resolution.x / nodes.data.scaler));
		ImGui::SliderFloat("node 1 Y", &nodes.data.position1.y, 0, resolution.y - (resolution.y / nodes.data.scaler));
		ImGui::SliderFloat("node 2 X", &nodes.data.position2.x, 0, resolution.x - (resolution.x / nodes.data.scaler));
		ImGui::SliderFloat("node 2 Y", &nodes.data.position2.y, 0, resolution.y - (resolution.y / nodes.data.scaler));
		ImGui::Separator();

		glm::vec2 normalizednode1 = glm::normalize(nodes.data.position1);
		glm::vec2 normalizednode2 = glm::normalize(nodes.data.position2);

		float normalizedDotProduct = glm::dot(normalizednode1, normalizednode2);
		float angleRadians = glm::acos(normalizedDotProduct);
		float angleDegrees = glm::degrees(angleRadians);
				
		ImGui::Text("normalized dot product: %.3f", normalizedDotProduct);
		ImGui::Text("angle in Radians: %.3f", angleRadians);
		ImGui::Text("angle in Degrees: %.3f", angleDegrees);
		ImGui::Separator();
		ImGui::Text("Dot Product pointers:");
		ImGui::TextColored(ImVec4(0, 0.66, 0, 1), "1 = same direction");
		ImGui::TextColored(ImVec4(0, 0.66, 0, 1), "0 = 90 degrees away (perpendicular)");
		ImGui::TextColored(ImVec4(0, 0.66, 0, 1), "-1 = completely behind");
		ImGui::TextColored(ImVec4(0, 0.66, 0, 1), "0.5 = 60 degrees away (perfect cone)");

		//make 2 children at the node locations
		ImGui::SetNextWindowPos(ImVec2(nodes.data.position1.x, nodes.data.position1.y));
		ImGui::SetNextWindowSize(ImVec2(resolution.x / nodes.data.scaler, resolution.y / nodes.data.scaler));
		ImGui::Begin("node 1 stats" , nullptr, ImGuiWindowFlags_NoDecoration);
		//just show the position and normalized values
		ImGui::TextColored(ImVec4(0, 0.66, 0.66, 1), "position:%i %i", (int)nodes.data.position1.x, (int)nodes.data.position1.y);
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0, 0.66, 0.66, 1), "norm:%.3f %.3f", normalizednode1.x, normalizednode1.y);
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(nodes.data.position2.x, nodes.data.position2.y));
		ImGui::SetNextWindowSize(ImVec2(resolution.x / nodes.data.scaler, resolution.y / nodes.data.scaler));
		ImGui::Begin("node 2 stats", nullptr, ImGuiWindowFlags_NoDecoration);
		//just show the position and normalized values?
		ImGui::TextColored(ImVec4(0, 0.66, 0.66, 1), "position:%i %i", (int)nodes.data.position2.x, (int)nodes.data.position2.y);
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0, 0.66, 0.66, 1), "norm:%.3f %.3f", normalizednode2.x, normalizednode2.y);
		ImGui::End();
	}

	void Resize(const tWindow* window, glm::ivec2 dimensions = glm::ivec2(0)) override
	{
		glm::vec2 resolution = glm::vec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
		glViewport(0, 0, resolution.x, resolution.y);
		defaultPayload.data.resolution = glm::ivec2(resolution.x, resolution.y);
		//let's try to set the projection so the 0 is at center

		defaultPayload.data.projection = glm::ortho(0.0f, resolution.x, resolution.y, 0.0f, 0.01f, 10.0f);

		defaultPayload.Update(gl_uniform_buffer, gl_dynamic_draw);
		defaultVertexBuffer.UpdateBuffer(glm::ivec2(defaultPayload.data.resolution / nodes.data.scaler));
	}
};