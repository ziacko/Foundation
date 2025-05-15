#include "texturedScene3D.h"

int main()
{
	texturedModel* exampleScene = new texturedModel();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}