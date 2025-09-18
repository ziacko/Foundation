#include "perlin3D.h"

int main()
{
	perlinScene3D* exampleScene = new perlinScene3D();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
