#include "computeTest.h"

int main()
{
	computeTestScene* exampleScene = new computeTestScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
