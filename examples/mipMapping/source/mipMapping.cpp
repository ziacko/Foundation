#include "mipMapping.h"

int main()
{
	mipMappingScene* exampleScene = new mipMappingScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
