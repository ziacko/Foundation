#include "TemporalUpsampling.h"

int main()
{
	temporalUpsampling* exampleScene = new temporalUpsampling();
	exampleScene->Initialize();
	exampleScene->Run();

	return 0;
}