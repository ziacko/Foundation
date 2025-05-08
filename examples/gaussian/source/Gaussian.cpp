#include "Gaussian.h"

int main()
{
	
	gaussianScene* exampleScene = new gaussianScene(bufferHandler_t<gaussianSettings_t>(), 
		new texture("textures/crate_sideup.png"));
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}