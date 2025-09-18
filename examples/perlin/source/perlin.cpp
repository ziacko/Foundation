#include "perlin.h"

int main()
{
	perlinScene* exampleScene = new perlinScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
