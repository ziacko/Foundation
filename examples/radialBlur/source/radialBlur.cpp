#include "radialBlur.h"

int main()
{
	radialBlurScene* exampleScene = new radialBlurScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
