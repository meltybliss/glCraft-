#include "Core/Application.h"
#include <memory>

int main(int argc, char** argv) {
	
	auto app = std::make_unique<Application>();

	if (!app->InitGL()) {
		return 1;
	}

	app->Run();

	return 0;
}