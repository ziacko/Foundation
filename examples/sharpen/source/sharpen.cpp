#include "sharpen.h"

int main()
{
	sharpenScene* exampleScene = new sharpenScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
