#ifndef FROST_H
#define FROST_H
#include <heatHaze.h>

class frostScene final : public heatHazeScene
{
public:
	explicit frostScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio ( frost )",
		const camera_t bubbleCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) : heatHazeScene(defaultTexture, windowName, bubbleCamera, shaderConfigPath)	{}

	~frostScene() override = default;

	void InitializeUniforms() override
	{
		heatHazeScene::InitializeUniforms();

		perlin->numOctaves = 5;

		perlin->uvScale = glm::vec2(10, 10);
		perlin->taylorInverse = 0.175;
		perlin->colorBias = 1.0f;
		perlin->patternValue1 = 1.075;
		perlin->patternValue2 = 1.257;
		bubble->offset = 0.033f;
	}
};

#endif
