
#pragma once
#include "SMAA.h"

struct resolutionSettings_t
{
    glm::vec2 resolutionScale{1.0f, 1.0f};

    explicit resolutionSettings_t(glm::vec2 res = glm::vec2(1.0f, 1.0f))
    {
        resolutionScale = res;
    }
};

class OAUpsamplerScene : public SMAAScene
{
public:

    explicit OAUpsamplerScene(const char* windowName = "Ziyad Barakat's portfolio (OAUpsampler)",
        const camera_t& camera = camera_t(defaultWindowSize, 0.31415f, camera_t::projection_e::perspective, 0.01f, 2000.f),
        const char* shaderConfigPath = SHADER_CONFIG_DIR,
        model_t model = model_t("models/SoulSpear/SoulSpear.fbx")) : SMAAScene(windowName, camera, shaderConfigPath, std::move(model))
    {
        resScale = glm::vec2(1, 1);
        scaledResolution = camera.resolution * resScale;
    }

    void Initialize() override
    {
        SMAAScene::Initialize();
    }

protected:

    glm::vec2 resScale{ 1, 1};
    glm::ivec2 scaledResolution{ resScale.x, resScale.y };
    bufferHandler_t<resolutionSettings_t> resolutionSettings;

    void GeometryPass() override
    {
        geometryBuffer.Bind();

        glDrawBuffers(1, &geometryBuffer.attachments["color"].FBODesc.attachmentFormat);

        //we just need the first LOd so only do the first 3 meshes
        for (size_t iter = 0; iter < 1; iter++)
        {
            testModel.meshes[iter].textures[0].SetActive(0);

            glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
            glUseProgram(geometryProgram->handle);

            glViewport(0, 0, scaledResolution.x, scaledResolution.y);

            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, nullptr);
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
        glViewport(0, 0, scaledResolution.x, scaledResolution.y);
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
        glViewport(0, 0, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
    }

    void SMAAPass() override
    {
        SMAABuffer.Bind();
        glDrawBuffers(1, &SMAABuffer.attachments["SMAA"].FBODesc.attachmentFormat);

        //current frame
        geometryBuffer.attachments["color"].SetActive(0); // color
        weightsBuffer.attachments["blend"].SetActive(1); //blending weights

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        glUseProgram(SMAAProgram->handle);
        glViewport(0, 0, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
    }

    void Update() override
    {
        SMAAScene::Update();
        resolutionSettings.Update(gl_uniform_buffer, gl_dynamic_draw);
    }

    void Draw() override
    {
        sceneCamera.ChangeProjection(camera_t::projection_e::perspective);
        sceneCamera.Update();
        UpdateDefaultBuffer();

        GeometryPass(); //render current scene with jitter

        sceneCamera.resolution = scaledResolution;
        sceneCamera.ChangeProjection(camera_t::projection_e::orthographic);
        sceneCamera.Update();
        UpdateDefaultBuffer();

        EdgeDetectionPass();
        BlendingWeightsPass();
        SMAAPass();

        sceneCamera.resolution = glm::vec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
        sceneCamera.Update();
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

        sceneCamera.resolution = resolution;
    }

    void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t> dimensions) override
    {
        scaledResolution = glm::vec2(dimensions.x, dimensions.y) * resScale;
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void HandleMaximize(const tWindow* window) override
    {
        scaledResolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resScale;
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void DrawResolutionSettings()
    {
        ImGui::Begin("resolution scale", &isGUIActive);
        if (ImGui::DragFloat("scaleX", &resolutionSettings.data.resolutionScale.x, 0.01f) ||
            ImGui::DragFloat("scaleY", &resolutionSettings.data.resolutionScale.y, 0.01f))
        {
            resScale =  resolutionSettings.data.resolutionScale;
            scaledResolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resScale;
            ResizeBuffers(scaledResolution);
        }
        ImGui::End();
    }

    void BuildGUI(tWindow* window, const ImGuiIO io) override
    {
        SMAAScene::BuildGUI(window, io);
        DrawResolutionSettings();
    }
};
