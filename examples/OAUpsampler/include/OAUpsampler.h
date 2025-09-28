
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
        const model_t& model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX")) : SMAAScene(windowName, camera, shaderConfigPath, model)
    {

        velocityUniforms = bufferHandler_t<reprojectSettings_t>();
        taaUniforms = bufferHandler_t<TAASettings_t>();
        jitterUniforms = bufferHandler_t<jitterSettings_t>();
        jitter2Uniforms = bufferHandler_t<jitter2Settings_t>();

        for (int iter = 0; iter < 128; iter++)
        {
            jitterUniforms.data.haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
        }
    }

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
    bufferHandler_t<resolutionSettings_t> resolutionSettings;

    bufferHandler_t<reprojectSettings_t> velocityUniforms;
    bufferHandler_t<TAASettings_t> taaUniforms;
    bufferHandler_t<jitterSettings_t> jitterUniforms;
    bufferHandler_t<jitter2Settings_t> jitter2Uniforms;

    //unscaled geometry buffer
    frameBuffer unscaledBuffer;

    // Track first frame so we can correctly seed previous matrices
    bool firstFrame = true;

    virtual void Update() override
    {
        SMAAScene::Update();

        jitterUniforms.Update();
        //jitter2Uniforms.Update();

        //if even frame then write to 1 and read from 0 and vice versa
        currentFrame = ((defaultPayload.data.totalFrames % 2) == 0) ? false : true;

        //resolutionSettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
    }

    void GeometryPass() override
    {
        GL_PUSH_DEBUG_GROUP();
        geometryBuffer.Bind();

        GLenum drawbuffers[2] = {
            geometryBuffer.attachments["color"].FBODesc.attachmentFormat, //color
            geometryBuffer.attachments["velocity"].FBODesc.attachmentFormat, //velocity
        };

        glDrawBuffers(2, drawbuffers);

        //we just need the first LOd so only do the first 3 meshes
        for (auto& mesh : testModel.meshes)
        {
            for (uint32_t texIter = 0; texIter < mesh.textures.size(); texIter++)
            {
                mesh.textures[texIter].SetActive(texIter);
            }

            glBindVertexArray(mesh.vertexArrayHandle);
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

        glDrawBuffers(1, &unscaledBuffer.attachments["color"].FBODesc.attachmentFormat);

        //we just need the first LOd so only do the first 3 meshes
        for (auto& mesh : testModel.meshes)
        {
            for (uint32_t texIter = 0; texIter < mesh.textures.size(); texIter++)
            {
                mesh.textures[texIter].SetActive(texIter);
            }

            glBindVertexArray(mesh.vertexArrayHandle);
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

        glDrawBuffers(1, &edgesBuffer.attachments["edge"].FBODesc.attachmentFormat);

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

        glDrawBuffers(1, &weightsBuffer.attachments["blend"].FBODesc.attachmentFormat);

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
        glDrawBuffers(1, &SMAABuffer.attachments["SMAA"].FBODesc.attachmentFormat);

        //current frame
        geometryBuffer.attachments["color"].SetActive(0); //color
        weightsBuffer.attachments["blend"].SetActive(1); //blending weights

        glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
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
        velocityUniforms.data.currentView =camera.view;
        camera.ChangeProjection(camera_t::projection_e::perspective);
        camera.Update();

        UpdateDefaultBuffer();

        GeometryPass();
        UnscaledPass();

        camera.ChangeProjection(camera_t::projection_e::orthographic);
        UpdateDefaultBuffer();

        EdgeDetectionPass();
        BlendingWeightsPass();
        SMAAPass();

        SMAAResolvePass();

        FinalPass(&historyFrames[currentFrame]->attachments["color"], &unscaledBuffer.attachments["color"]);

        DrawGUI(window);

        manager->SwapDrawBuffers(window);
        ClearBuffers();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void ClearBuffers() override
    {
        GL_PUSH_DEBUG_GROUP();
        //ok copy the current frame into the previous frame and clear the rest of the buffers
        float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
        float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

      	historyFrames[!currentFrame]->Bind(); //clear the previous, the next frame current becomes previous
      	historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("color"), clearColor1);
      	historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("depth"), clearColor2);
      	//copy current depth to previous or vice versa?
      	historyFrames[currentFrame]->GetAttachmentRef("depth").Copy(&geometryBuffer.GetAttachmentRef("depth")); //copy depth over

      	glClear(GL_DEPTH_BUFFER_BIT);
      	historyFrames[!currentFrame]->Unbind();

        geometryBuffer.Bind();
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("color"), clearColor1);
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("velocity"), clearColor2);
        geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("depth"), clearColor2);
        glClear(GL_DEPTH_BUFFER_BIT);
        geometryBuffer.Unbind();

        SMAABuffer.Bind();
        SMAABuffer.ClearTexture(SMAABuffer.GetAttachmentRef("SMAA"), clearColor1);
        SMAABuffer.Unbind();

        edgesBuffer.Bind();
        edgesBuffer.ClearTexture(edgesBuffer.GetAttachmentRef("edge"), clearColor2);
        edgesBuffer.Unbind();

        weightsBuffer.Bind();
        weightsBuffer.ClearTexture(weightsBuffer.GetAttachmentRef("blend"), clearColor2);
        weightsBuffer.Unbind();

        unscaledBuffer.Bind();
        unscaledBuffer.ClearTexture(unscaledBuffer.GetAttachmentRef("color"), clearColor1);
        glClear(GL_DEPTH_BUFFER_BIT);
        unscaledBuffer.Unbind();

    // Keep camera in a known projection for the start of next frame but DO NOT update velocity uniforms here.
        camera.ChangeProjection(camera_t::projection_e::perspective);
        velocityUniforms.data.previousProjection =camera.projection;
        velocityUniforms.data.previousView =camera.view;
        velocityUniforms.data.prevTranslation = testModel.makeTransform(); //could be jittering the camera instead of the geometry?
        velocityUniforms.Update();
        glPopDebugGroup();
    }

    virtual void InitializeUniforms() override
    {
        SMAAScene::InitializeUniforms();

        jitterUniforms.Initialize(2);
        velocityUniforms.Initialize(3);
        taaUniforms.Initialize(4);
        resolutionSettings.Initialize(5);
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
        scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.data.resolutionScale.x,
            window->GetSettings().resolution.height * resolutionSettings.data.resolutionScale.y);
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void HandleMaximize(const tWindow* window) override
    {
        scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.data.resolutionScale.x,
            window->GetSettings().resolution.height * resolutionSettings.data.resolutionScale.y);
        ResizeBuffers(glm::ivec2(scaledResolution));
    }

    void DrawResolutionSettings()
    {
        if (ImGui::BeginTabItem("resolution scale"))
        {
            if (ImGui::DragFloat("scaleX", &resolutionSettings.data.resolutionScale.x, 0.01f, 0.1f, 2.0f) ||
                ImGui::DragFloat("scaleY", &resolutionSettings.data.resolutionScale.y, 0.01f, 0.1f, 2.0f))
            {
                scaledResolution = glm::ivec2(window->GetSettings().resolution.width * resolutionSettings.data.resolutionScale.x,
                    window->GetSettings().resolution.height * resolutionSettings.data.resolutionScale.y);
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
