#include "heatHaze.h"

int main()
{
	heatHazeScene* exampleScene = new heatHazeScene();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}
