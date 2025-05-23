#include "gaussianMulti.h"

int main()
{
	gaussianMultiScene* exampleScene = new gaussianMultiScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
