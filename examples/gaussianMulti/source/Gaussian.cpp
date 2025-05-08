#include "GaussianMulti.h"

int main()
{
	
	gaussianMultiScene* exampleScene = new gaussianMultiScene(new texture("textures/crate_sideup.png"));
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}