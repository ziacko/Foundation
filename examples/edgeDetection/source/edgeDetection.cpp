#include "edgeDetection.h"

int main()
{
	edgeDetectionScene* exampleScene = new edgeDetectionScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
