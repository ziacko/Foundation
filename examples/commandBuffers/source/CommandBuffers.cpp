#include "CommandBuffers.h"

int main()
{
	commandScene* command = new commandScene();
	command->Initialize();
	command->Run();

	return 0;
}
