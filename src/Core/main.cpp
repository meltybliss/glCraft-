#include "Core/Application.h"

int main() {
	
	Application app;

	if (!app.InitGL()) {
		return 1;
	}

	app.Run();

	return 0;
}