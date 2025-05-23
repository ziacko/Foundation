#include "FeedbackBuffers.h"

int main()
{
	feedbackScene* feedback = new feedbackScene();
	feedback->Initialize();
	feedback->Run();

	return 0;
}
