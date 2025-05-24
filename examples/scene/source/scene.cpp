#include <scene.h>
int main()
{
	sceneDesciptor_t sceneDesc;
	scene* exampleScene = new scene(sceneDesc);
	exampleScene->Initialize();
	exampleScene->Run();
	return 0;
}
