#include "Editor.h"

int main(int argc, char* argv[])
{
	Editor Editor;

	if (!Editor.Initialize())
	{
		return 1;
	}

	while (Editor.PollEvents())
	{
		Editor.Update();
		Editor.RenderFrame();
	}

	Editor.Destroy();

	return 0;
}