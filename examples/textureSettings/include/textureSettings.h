#ifndef TEXTURE_SETTINGS_H
#define TEXTURE_SETTINGS_H
#include <textured.h>

using magFilterSettings_t = enum { LINEAR = 0, NEAREST };
using minFilterSettings_t = enum { NEAREST_MIPMAP_NEAREST = 2, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_NEAREST, LINEAR_MIPMAP_LINEAR };
using wrapSettings_t = enum { CLAMP_TO_EDGE = 0, MIRROR_CLAMP_TO_EDGE, CLAMP_TO_BORDER, REPEAT, MIRRORED_REPEAT };

//no need to edit these. but I can't make these const :(
constexpr std::array<const char*, 5>		wrapSettings = { "clamp to edge", "mirror clamp to edge", "clamp to border", "repeat", "mirrored repeat" };
constexpr std::array<const char*, 2>		magFilterSettings = { "linear", "nearest" };
constexpr std::array<const char*, 6>		minFilterSettings = { "linear", "nearest", "nearest mipmap nearest" , "nearest mipmap linear" , "linear mipmap nearest" , "linear mipmap linear" };

class textureSettingsScene : public texturedScene
{
public:
	explicit textureSettingsScene(const texture defaultTexture = texture("textures/crate_sideup.png"),
	                              const char* windowName = "Ziyad Barakat's Portfolio (texture settings)",
	                              const camera_t textureCamera = camera_t(),
	                              const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath) {}

	virtual void Initialize() override
	{
		defaultTexture.texDesc.hasMips = true;
		defaultTexture.texDesc.levels = 10;
		texturedScene::Initialize();
	}

	void InitializeUniforms() override
	{
		mip.Initialize(1);
		texturedScene::InitializeUniforms();
	}

	void Update() override
	{
		texturedScene::Update();
		mip.Update();
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		DrawTextureSettings();
	}

	virtual void DrawTextureSettings()
	{
		if (ImGui::BeginTabItem("texture settings"))
		{
			if (ImGui::ListBox("mag filter setting", &magFilterIndex, magFilterSettings.data(), magFilterSettings.size()))
			{
				defaultTexture.SetMagFilter(filterMap[magFilterIndex]);
			}

			//min
			if (ImGui::ListBox("min filter setting", &minFilterIndex, minFilterSettings.data(), minFilterSettings.size()))
			{
				defaultTexture.SetMinFilter(filterMap[minFilterIndex]);
			}

			//S wrap setting
			if (ImGui::ListBox("S wrap texture setting", &sWrapIndex, wrapSettings.data(), wrapSettings.size()))
			{
				defaultTexture.SetWrapS(wrapMap[sWrapIndex]);
			}
			//T wrap setting
			if (ImGui::ListBox("T wrap texture setting", &tWrapIndex, wrapSettings.data(), wrapSettings.size()))
			{
				defaultTexture.SetWrapT(wrapMap[tWrapIndex]);
			}
			//R wrap setting (3D textures) //not doing 3D right now so I'll leave it out
			/*if (ImGui::ListBox("R wrap texture setting", &rWrapIndex, wrapSettings.data(), wrapSettings.size()))
			{
				defaultTexture->SetWrapR(rWrapIndex);
			}*/

			ImGui::SliderFloat("mip level", &mip.data, 0, 10);
			ImGui::EndTabItem();
		}
	}

protected:

	bufferHandler_t<float>	mip;

	typedef std::pair<uint32_t, uint32_t> paramEntry;
	static tsl::robin_map<uint32_t, uint32_t> filterMap;
	static tsl::robin_map<uint32_t, uint32_t> wrapMap;

	magFilterSettings_t				minFilterSetting = LINEAR;
	magFilterSettings_t				magFilterSetting = LINEAR;
	wrapSettings_t					wrapSSetting = CLAMP_TO_EDGE;
	wrapSettings_t					wrapTSetting = CLAMP_TO_EDGE;
	wrapSettings_t					wrapRSetting = CLAMP_TO_EDGE;

	//look into making these into std::pairs?
	int minFilterIndex = 0; //can't be a local variable
	int magFilterIndex = 0;

	int sWrapIndex = 0;
	int tWrapIndex = 0;
	int rWrapIndex = 0;
};

tsl::robin_map<uint32_t, uint32_t> textureSettingsScene::filterMap =
{
	paramEntry(LINEAR, GL_LINEAR),
	paramEntry(NEAREST, GL_NEAREST),
	paramEntry(NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST),
	paramEntry(NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR),
	paramEntry(LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ,
	paramEntry(LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST),
};

tsl::robin_map<uint32_t, uint32_t> textureSettingsScene::wrapMap =
{
	paramEntry(CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE),
	paramEntry(MIRROR_CLAMP_TO_EDGE, GL_MIRROR_CLAMP_TO_EDGE),
	paramEntry(CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER),
	paramEntry(REPEAT, GL_REPEAT),
	paramEntry(MIRRORED_REPEAT, GL_MIRRORED_REPEAT)
};

#endif
