
#pragma once
#include "SMAA.h"

struct jitterSettings_t
{
    glm::vec2			haltonSequence[128];
    float				haltonScale;
    int					haltonIndex;
    int					enableDithering;
    float				ditheringScale;

    jitterSettings_t()
    {
        haltonIndex = 16;
        enableDithering = 1;
        haltonScale = 10.0f;
        ditheringScale = 0.0f;
    }

    ~jitterSettings_t() {};
};

struct jitter2Settings_t
{
    glm::vec4		indices[2];
    glm::vec2		samples[2];
    float			scale;
    unsigned int	numSamples;

    jitter2Settings_t()
    {
        scale = 100.0f;
        numSamples = 2;

        samples[0] = glm::vec2(0.25f, 0.25f);
        samples[1] = glm::vec2(-0.25f, -0.25f);
        indices[0] = glm::vec4(1, 1, 1, 0);
        indices[1] = glm::vec4(2, 2, 2, 0);
    }

    ~jitter2Settings_t() {};
};

struct reprojectSettings_t
{
    glm::mat4		previousProjection;
    glm::mat4		previousView;
    glm::mat4		prevTranslation;

    glm::mat4		currentView;

    reprojectSettings_t()
    {
        this->previousProjection = glm::mat4(1.0f);
        this->previousView = glm::mat4(1.0f);
        this->prevTranslation = glm::mat4(1.0f);

        this->currentView = glm::mat4(1.0f);
    }

    ~reprojectSettings_t() {};
};

struct TAASettings_t
{
    //velocity
    float velocityScale;
    //Inside
    float feedbackFactor;
    //Custom
    float maxDepthFalloff;

    TAASettings_t()
    {
        this->feedbackFactor = 0.9f;
        this->maxDepthFalloff = 1.0f;
        this->velocityScale = 1.0f;
    }

    ~TAASettings_t() { };
};

constexpr glm::vec2 defaultResScale = glm::vec2(1, 1);

struct resolutionSettings_t
{
    glm::vec2 resolutionScale{defaultResScale};

    explicit resolutionSettings_t(const glm::vec2& res = defaultResScale)
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
        const model_t& model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX")) : SMAAScene(windowName, camera, shaderConfigPath, model)
    {
        jitterUniforms = new jitterSettings_t();
        for (int iter = 0; iter < 128; iter++)
        {
            jitterUniforms->haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
        }
    }

    ~OAUpsamplerScene() override { delete jitterUniforms; };

    void Initialize() override
    {
        scene3D::Initialize();
        SMAAArea.LoadTexture();
        SMAASearch.LoadTexture();

        scaledResolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

        geometryBuffer.Initialize();
        geometryBuffer.Bind();
        geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));

        FBODescriptor velDesc;
        velDesc.format = GL_RG;
        velDesc.internalFormat = GL_RG16_SNORM;
        velDesc.dataType = GL_FLOAT;
        velDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

        geometryBuffer.AddAttachment(frameBuffer::attachment_t("velocity", velDesc));

        FBODescriptor depthDesc;
        depthDesc.dataType = GL_FLOAT;
        depthDesc.format = GL_DEPTH_COMPONENT;
        depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
        depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
        depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

        geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

        edgesBuffer.Initialize();
        edgesBuffer.Bind();

        FBODescriptor edgeDesc;
        edgeDesc.format = GL_RG;
        edgeDesc.internalFormat = GL_RG8;
        edgeDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

        edgesBuffer.AddAttachment(frameBuffer::attachment_t("edge", edgeDesc));

        weightsBuffer.Initialize();
        weightsBuffer.Bind();

        weightsBuffer.AddAttachment(frameBuffer::attachment_t("blend", colorDesc));

        SMAABuffer.Initialize();
        SMAABuffer.Bind();
        SMAABuffer.AddAttachment(frameBuffer::attachment_t("SMAA", colorDesc));

        for(unsigned int iter = 0; iter < numPreviousFrames; iter++)
        {
            frameBuffer* newBuffer = new frameBuffer();

            newBuffer->Initialize();
            newBuffer->Bind();
            newBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
            newBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

            historyFrames.push_back(newBuffer);
           // delete newBuffer;
        }

        unscaledBuffer.Initialize();
        unscaledBuffer.Bind();

        unscaledBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
        unscaledBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

        defProgram = shaderProgramsMap["geometry"];
        edgeDetectionProgram = shaderProgramsMap["edgeDetection"];
        blendingWeightProgram = shaderProgramsMap["blendingWeight"];
        SMAAProgram = shaderProgramsMap["neighborhoodBlend"];
        resolveProgram = shaderProgramsMap["resolve"];
        compareProgram = shaderProgramsMap["compare"];
        finalProgram = shaderProgramsMap["final"];

        frameBuffer::Unbind();
    }

protected:

    std::vector<frameBuffer*>		historyFrames;

    shaderProgram_t resolveProgram;
    uint8_t numPreviousFrames = 2;

    bool enableCompare = true;
    bool currentFrame = false;

    glm::ivec2 scaledResolution;

    resolutionSettings_t   resolutionSettings;
    reprojectSettings_t*    reprojectUniforms;
    TAASettings_t*          taaUniforms;
    jitterSettings_t*       jitterUniforms;
    jitter2Settings_t*      jitter2Uniforms;

    //unscaled geometry buffer
    frameBuffer unscaledBuffer;

    // Track first frame so we can correctly seed previous matrices
    bool firstFrame = true;

    virtual void Update() override
    {
        SMAAScene::Update();

        //if even frame then write to 1 and read from 0 and vice versa
        currentFrame = ((defaultPayload->totalFrames % 2) == 0) ? false : true;

        //resolutionSettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
    }

    void GeometryPass() override
    {
        GL_PUSH_DEBUG_GROUP();
        geometryBuffer.Bind();
        geometryBuffer.DrawAll();

        //we just need the first LOd so only do the first 3 meshes
        for (auto& mesh : testModel.meshes)
        {
            for (uint32_t texIter = 0; texIter < mesh.textures.size(); texIter++)
            {
                mesh.textures[texIter].SetActive(texIter);
            }

            mesh.Bind();
            defProgram.Use();

            glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
            glCullFace(GL_BACK);

            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        frameBuffer::Unbind();
        glPopDebugGroup();
    }

    void UnscaledPass()
    {
        GL_PUSH_DEBUG_GROUP();
        unscaledBuffer.Bind();

        unscaledBuffer.attachments["color"].Draw();

        //we just need the first LOd so only do the first 3 meshes
        for (auto& mesh : testModel.meshes)
        {
            for (uint32_t texIter = 0; texIter < mesh.textures.size(); texIter++)
            {
                mesh.textures[texIter].SetActive(texIter);
            }

            mesh.Bind();
            defProgram.Use();

            glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

            if (wireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        frameBuffer::Unbind();
        glPopDebugGroup();
    }

    void EdgeDetectionPass() override
    {
        GL_PUSH_DEBUG_GROUP();
        edgesBuffer.Bind();
        edgesBuffer.attachments["edge"].Draw();

        geometryBuffer.attachments["color"].SetActive(0);//color
        geometryBuffer.attachments["depth"].SetActive(1);//depth

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        edgeDetectionProgram.Use();
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
        glPopDebugGroup();
    }

    void BlendingWeightsPass() override
    {
        GL_PUSH_DEBUG_GROUP();
        weightsBuffer.Bind();

        weightsBuffer.attachments["blend"].Draw();

        edgesBuffer.attachments["edge"].SetActive(0);
        SMAAArea.SetActive(1);
        SMAASearch.SetActive(2);

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        blendingWeightProgram.Use();
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
        glPopDebugGroup();
    }

    void SMAAPass() override
    {
        GL_PUSH_DEBUG_GROUP();
        SMAABuffer.Bind();
        SMAABuffer.attachments["SMAA"].Draw();

        //current frame
        geometryBuffer.attachments["color"].SetActive(0); //color
        weightsBuffer.attachments["blend"].SetActive(1); //blending weights

        defaultVertexBuffer.Bind();
        SMAAProgram.Use();
        glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        frameBuffer::Unbind();
        glPopDebugGroup();
    }

    virtual void SMAAResolvePass()
    {
        GL_PUSH_DEBUG_GROUP();
        historyFrames[currentFrame]->Bind();
        GLenum drawBuffers[1] = {
            historyFrames[currentFrame]->attachments["color"].FBODesc.attachmentFormat
        };
        glDrawBuffers(1, drawBuffers);

        SMAABuffer.attachments["SMAA"].SetActive(0); //current color
        geometryBuffer.attachments["depth"].SetActive(1);//current depth

        historyFrames[!currentFrame]->attachments["color"].SetActive(2); //previous color
        historyFrames[!currentFrame]->attachments["depth"].SetActive(3);//previous depth

        geometryBuffer.attachments["velocity"].SetActive(4); //velocity

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
        resolveProgram.Use();
        glViewport(0, 0, scaledResolution.x, scaledResolution.y);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        historyFrames[currentFrame]->Unbind();
        glPopDebugGroup();
    }

    void Draw() override
    {
        reprojectUniforms->currentView = camera.view;
        camera.ChangeProjection(camera_t::projection_e::perspective);
        camera.Update();

        UpdateDefaultUniforms(camera, clock, &testModel);

        //enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GeometryPass();
        UnscaledPass();

        glDisable(GL_BLEND);

        camera.ChangeProjection(camera_t::projection_e::orthographic);
        UpdateDefaultUniforms(camera, clock, &testModel);

        EdgeDetectionPass();
        BlendingWeightsPass();
        SMAAPass();

        SMAAResolvePass();

        FinalPass(&historyFrames[currentFrame]->attachments["color"], &unscaledBuffer.attachments["color"]);
    }

    void ClearBuffers() override
    {
        GL_PUSH_DEBUG_GROUP();

        historyFrames[!currentFrame]->Bind(); //clear the previous, the next frame current becomes previous
        historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("color"), clearColor);
        historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("depth"), clearDepth);
        //copy current depth to previous or vice versa?
        historyFrames[currentFrame]->GetAttachmentRef("depth").Copy(&geometryBuffer.GetAttachmentRef("depth")); //copy depth over
       // historyFrames[currentFrame]->GetAttachmentRef("color").Copy(&historyFrames[!currentFrame]->GetAttachmentRef("color")); //copy depth over

        glClear(GL_DEPTH_BUFFER_BIT );
        historyFrames[!currentFrame]->Unbind();

        geometryBuffer.Bind();
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("color"), clearColor);
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("velocity"), clearDepth);
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("depth"), clearDepth);
        glClear(GL_DEPTH_BUFFER_BIT);
        geometryBuffer.Unbind();

        SMAABuffer.Bind();
        SMAABuffer.ClearTexture(SMAABuffer.GetAttachmentRef("SMAA"), clearColor);
        glClear(GL_DEPTH_BUFFER_BIT);
        SMAABuffer.Unbind();

        edgesBuffer.Bind();
        edgesBuffer.ClearTexture(edgesBuffer.GetAttachmentRef("edge"), clearDepth);
        edgesBuffer.Unbind();

        weightsBuffer.Bind();
        weightsBuffer.ClearTexture(weightsBuffer.GetAttachmentRef("blend"), clearDepth);
        weightsBuffer.Unbind();

        unscaledBuffer.Bind();
        unscaledBuffer.ClearTexture(unscaledBuffer.GetAttachmentRef("color"), clearColor);
        glClear(GL_DEPTH_BUFFER_BIT);
        unscaledBuffer.Unbind();

        // Keep camera in a known projection for the start of next frame but DO NOT update velocity uniforms here.
        camera.ChangeProjection(camera_t::projection_e::perspective);
        reprojectUniforms->previousProjection =camera.projection;
        reprojectUniforms->previousView =camera.view;
        reprojectUniforms->prevTranslation = testModel.makeTransform(); //could be jittering the camera instead of the geometry?
        glPopDebugGroup();
    }

    virtual void InitializeUniforms() override
    {
        SMAAScene::InitializeUniforms();

        auto taaBlock = &bufferHandler.uniformBlocks["taaSettings"];
        taaBlock->SetPayload(TAASettings_t());
        taaUniforms = taaBlock->GetPayload<TAASettings_t>();

        //auto sharpenBlock = &bufferHandler.uniformBlocks["sharpenSettings"];
        //sharpenBlock->SetPayload<sharpenSettings_t>(sharpenSettings_t());
        //sharpenSettings = sharpenBlock->GetPayload<sharpenSettings_t>();

        auto jitterBlock = &bufferHandler.uniformBlocks["jitterSettings"];
        jitterBlock->SetPayload(*jitterUniforms);
        jitterUniforms = jitterBlock->GetPayload<jitterSettings_t>();

        auto reprojectBlock = &bufferHandler.uniformBlocks["reprojectSettings"];
        reprojectBlock->SetPayload(reprojectSettings_t());
        reprojectUniforms = reprojectBlock->GetPayload<reprojectSettings_t>();
    }

    void ResizeBuffers(const glm::ivec2 resolution) override
    {
        for (auto frame : historyFrames)
        {
            for (auto iter : frame->attachments | std::views::values)
            {
                frame->GetAttachmentRef(iter.uniformName).Resize(glm::ivec3(resolution, 1));
            }
        }
        for (auto val : geometryBuffer.attachments | std::views::values)
        {
            val.Resize(glm::ivec3(resolution, 1));
        }

        for (auto val : edgesBuffer.attachments | std::views::values)
        {
            val.Resize(glm::ivec3(resolution, 1));
        }

        for (auto val : weightsBuffer.attachments | std::views::values)
        {
            val.Resize(glm::ivec3(resolution, 1));
        }

        for (auto val : SMAABuffer.attachments | std::views::values)
        {
            val.Resize(glm::ivec3(resolution, 1));
        }

        for (auto val : unscaledBuffer.attachments | std::views::values)
        {
            unscaledBuffer.GetAttachmentRef(val.uniformName).Resize(glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1));
        }
    }

    void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions) override
    {
        scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.resolutionScale.x,
            window->GetSettings().resolution.height * resolutionSettings.resolutionScale.y);
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void HandleMaximize(const tWindow* window) override
    {
        scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.resolutionScale.x,
            window->GetSettings().resolution.height * resolutionSettings.resolutionScale.y);
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void DrawResolutionSettings()
    {
        if (ImGui::BeginTabItem("resolution scale"))
        {
            if (ImGui::DragFloat("scaleX", &resolutionSettings.resolutionScale.x, 0.01f, 0.1f, 2.0f) ||
                ImGui::DragFloat("scaleY", &resolutionSettings.resolutionScale.y, 0.01f, 0.1f, 2.0f))
            {
                scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.resolutionScale.x,
                    window->GetSettings().resolution.height * resolutionSettings.resolutionScale.y);
                ResizeBuffers(scaledResolution);
            }
            ImGui::EndTabItem();
        }
    }

    virtual void DrawTAASettings()
    {
        if(ImGui::BeginTabItem("TAA Settings"))
        {
            ImGui::Checkbox("enable Compare", &enableCompare);
            ImGui::SliderFloat("feedback factor", &taaUniforms->feedbackFactor, 0.0f, 1.0f);
            ImGui::InputFloat("max depth falloff", &taaUniforms->maxDepthFalloff, 0.01f);

            //velocity settings
            ImGui::Separator();
            //ImGui::SameLine();
            ImGui::Text("Velocity settings");
            ImGui::SliderFloat("Velocity scale", &taaUniforms->velocityScale, 0.0f, 10.0f);

            //jitter settings
            ImGui::Separator();
            //ImGui::SameLine();
            ImGui::DragFloat("halton scale", &jitterUniforms->haltonScale, 0.1f, 0.0f, 15.0f, "%.3f");
            ImGui::DragInt("halton index", &jitterUniforms->haltonIndex, 1.0f, 0, 128);
            ImGui::DragInt("enable dithering", &jitterUniforms->enableDithering, 1.0f, 0, 1);
            ImGui::DragFloat("dithering scale", &jitterUniforms->ditheringScale, 1.0f, 0.0f, 1000.0f, "%.3f");

            ImGui::EndTabItem();
        }
    }

    void BuildGUI(tWindow* window, const ImGuiIO& io) override
    {
        SMAAScene::BuildGUI(window, io);
        DrawResolutionSettings();
        DrawTAASettings();
    }

    float CreateHaltonSequence(unsigned int index, int base)
    {
        float f = 1;
        float r = 0;
        int current = index;
        do
        {
            f = f / base;
            r = r + f * (current % base);
            current = (int)glm::floor(current / base);
        } while (current > 0);

        return r;
    }
};
