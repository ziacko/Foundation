
#pragma once
#include "SMAA.h"

constexpr glm::vec2 defaultResScale = glm::vec2(1, 1);

struct resolutionSettings_t
{
    glm::vec2 resolutionScale{defaultResScale};

    resolutionSettings_t(const glm::vec2& res = defaultResScale)
    {
        resolutionScale = res;
    }
};

class OAUpsamplerScene final : public SMAAScene
{
public:

    OAUpsamplerScene(const char* windowName = "Ziyad Barakat's portfolio (OAUpsampler)",
        const camera_t& camera = camera_t(defaultWindowSize, defaultCameraSpeed, camera_t::projection_e::perspective),
        const char* shaderConfigPath = "SMAA",
        const model_t& model = model_t("models/SoulSpear/SoulSpear.fbx")) : SMAAScene(windowName, camera, shaderConfigPath, model)
    {
        resolutionSettings = resolutionSettings_t(defaultResScale);
        resScale = glm::vec2(defaultResScale);
		glm::vec2 windowResolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
        scaledResolution = windowResolution * resScale;
    }

    void Initialize() override
    {
        SMAAScene::Initialize();

    }

protected:

    glm::vec2 resScale{defaultResScale};
    glm::ivec2 scaledResolution{ resScale.x, resScale.y };
    bufferHandler_t<resolutionSettings_t> resolutionSettings;

    void GeometryPass() override
    {
        geometryBuffer.Bind();

        glDrawBuffers(1, &geometryBuffer.attachments["color"].FBODesc.attachmentFormat);

        //we just need the first LOd so only do the first 3 meshes
        for (auto& mesh : testModel.meshes)
        {
            for (uint32_t texIter = 0; texIter < mesh.textures.size(); texIter++)
            {
                mesh.textures[texIter].SetActive(texIter);
            }

            glBindVertexArray(mesh.vertexArrayHandle);
            glUseProgram(geometryProgram->handle);

            glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);

            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        frameBuffer::Unbind();
    }

    void EdgeDetectionPass() override
    {
        edgesBuffer.Bind();

        glDrawBuffers(1, &edgesBuffer.attachments["edge"].FBODesc.attachmentFormat);

        geometryBuffer.attachments["color"].SetActive(0);//color
        geometryBuffer.attachments["depth"].SetActive(1);//depth

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        glUseProgram(edgeDetectionProgram->handle);
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
    }

    void BlendingWeightsPass() override
    {
        weightsBuffer.Bind();

        glDrawBuffers(1, &weightsBuffer.attachments["blend"].FBODesc.attachmentFormat);

        edgesBuffer.attachments["edge"].SetActive(0);
        SMAAArea.SetActive(1);
        SMAASearch.SetActive(2);

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        glUseProgram(blendingWeightProgram->handle);
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
    }

    void SMAAPass() override
    {
        SMAABuffer.Bind();
        glDrawBuffers(1, &SMAABuffer.attachments["SMAA"].FBODesc.attachmentFormat);

        //current frame
        geometryBuffer.attachments["color"].SetActive(0); //color
        weightsBuffer.attachments["blend"].SetActive(1); //blending weights

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        glUseProgram(SMAAProgram->handle);
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
    }

    void Update() override
    {
        SMAAScene::Update();
        resolutionSettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
    }

    void Draw() override
    {
        camera.ChangeProjection(camera_t::projection_e::perspective);
        camera.Update();
        UpdateDefaultBuffer();

        GeometryPass();

        camera.resolution = scaledResolution;
        camera.ChangeProjection(camera_t::projection_e::orthographic);
        camera.Update();
        UpdateDefaultBuffer();

        EdgeDetectionPass();
        BlendingWeightsPass();
        SMAAPass();

        camera.resolution = glm::vec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
        camera.Update();
        UpdateDefaultBuffer();
        FinalPass(&SMAABuffer.attachments["SMAA"], &geometryBuffer.attachments["color"]);

        DrawGUI(window);

        manager->SwapDrawBuffers(window);
        ClearBuffers();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void InitializeUniforms() override
    {
        SMAAScene::InitializeUniforms();
        resolutionSettings.Initialize(2);
    }

    void ResizeBuffers(const glm::ivec2 resolution) override
    {
        for (auto val : geometryBuffer.attachments | std::views::values)
        {
            val.Resize(resolution);
        }

        for (auto val : edgesBuffer.attachments | std::views::values)
        {
            val.Resize(resolution);
        }

        for (auto val : weightsBuffer.attachments | std::views::values)
        {
            val.Resize(resolution);
        }

        for (auto val : SMAABuffer.attachments | std::views::values)
        {
            val.Resize(resolution);
        }
    }

    void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions) override
    {
        UpdateResolution(glm::ivec2(dimensions.x, dimensions.y));
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void HandleMaximize(const tWindow* window) override
    {
        UpdateResolution(glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height));
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void DrawResolutionSettings()
    {
        if (ImGui::BeginTabItem("resolution scale"))
        {
            if (ImGui::DragFloat("scaleX", &resolutionSettings.data.resolutionScale.x, 0.01f, 0.1f, 2.0f) ||
                ImGui::DragFloat("scaleY", &resolutionSettings.data.resolutionScale.y, 0.01f, 0.1f, 2.0f))
            {
                UpdateResolutionScale(resolutionSettings.data.resolutionScale);
                ResizeBuffers(scaledResolution);
            }
            ImGui::EndTabItem();
        }
    }

    void BuildGUI(tWindow* window, const ImGuiIO& io) override
    {
        SMAAScene::BuildGUI(window, io);
        DrawResolutionSettings();
    }

    void UpdateResolutionScale(const glm::vec2& resolutionScale)
    {
        resScale =  resolutionScale;
        scaledResolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resScale;
        camera.resolution = scaledResolution;
        camera.Update();

        SMAASettings.data.rtMetrics = glm::vec4(1.0 / scaledResolution.x, 1.0 / scaledResolution.y, scaledResolution.x, scaledResolution.y);
    }

    void UpdateResolution(const glm::ivec2& resolution)
    {
        scaledResolution = glm::vec2(resolution.x, resolution.y) * resScale;
        camera.resolution = scaledResolution;
        camera.Update();

        SMAASettings.data.rtMetrics = glm::vec4(1.0 / scaledResolution.x, 1.0 / scaledResolution.y, scaledResolution.x, scaledResolution.y);
    }
};
