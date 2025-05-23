#include "depthPrePass.h"

int main()
{
	depthPrePassScene* exampleScene = new depthPrePassScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
